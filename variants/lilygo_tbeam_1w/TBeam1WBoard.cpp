#include "TBeam1WBoard.h"

// How long to keep the fan running after the last TX before switching it off
// during RX, to let the 1W PA dissipate residual heat. Placeholder value -
// tune based on real thermal testing (heavy TX duty cycle vs ambient temp).
static constexpr uint32_t FAN_TX_COOLDOWN_MS = 30000;

void TBeam1WBoard::begin() {
  ESP32Board::begin();

  // Power on radio module (must be done before radio init)
  pinMode(SX126X_POWER_EN, OUTPUT);
  digitalWrite(SX126X_POWER_EN, HIGH);
  radio_powered = true;
  delay(10);  // Allow radio to power up

  // RF switch RXEN pin handled by RadioLib via setRfSwitchPins()

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize fan control (on by default - 1W PA can overheat)
  pinMode(FAN_CTRL_PIN, OUTPUT);
  digitalWrite(FAN_CTRL_PIN, HIGH);
}

void TBeam1WBoard::onBeforeTransmit() {
  // LNA must be off during TX regardless of the FEM gain setting - this is
  // unconditional, not gated by _fem_lna_enabled, to protect the LNA from
  // the PA's output (DIO2 handles the PA-side switching separately).
  digitalWrite(SX126X_RXEN, LOW);
  setFanEnabled(true);   // PA is about to generate heat
  _last_tx_millis = millis();
  digitalWrite(LED_PIN, HIGH);  // TX LED on
}

void TBeam1WBoard::onAfterTransmit() {
  digitalWrite(LED_PIN, LOW);   // TX LED off
}

void TBeam1WBoard::onBeforeReceive() {
  digitalWrite(SX126X_RXEN, _fem_lna_enabled ? HIGH : LOW);
  if (millis() - _last_tx_millis > FAN_TX_COOLDOWN_MS) {
    setFanEnabled(false);  // idle listening, PA cooled down - save power
  }
}

bool TBeam1WBoard::setLoRaFemLnaEnabled(bool enable) {
  _fem_lna_enabled = enable;
  return true;
}

bool TBeam1WBoard::canControlLoRaFemLna() const {
  return true;
}

bool TBeam1WBoard::isLoRaFemLnaEnabled() const {
  return _fem_lna_enabled;
}

uint16_t TBeam1WBoard::getBattMilliVolts() {
  // T-Beam 1W uses 7.4V battery with voltage divider
  // ADC reads through divider - adjust multiplier based on actual divider ratio
  analogReadResolution(12);
  uint32_t raw = 0;
  for (int i = 0; i < 8; i++) {
    raw += analogRead(BATTERY_PIN);
  }
  raw = raw / 8;
  // Assuming voltage divider ratio from ADC_MULTIPLIER
  // 3.3V reference, 12-bit ADC (4095 max)
  return static_cast<uint16_t>((raw * 3300 * ADC_MULTIPLIER) / 4095);
}

const char* TBeam1WBoard::getManufacturerName() const {
  return "LilyGo T-Beam 1W";
}

void TBeam1WBoard::powerOff() {
  // Turn off radio LNA (CTRL pin must be LOW when not receiving)
  digitalWrite(SX126X_RXEN, LOW);

  // Turn off radio power
  digitalWrite(SX126X_POWER_EN, LOW);
  radio_powered = false;

  // Turn off LED and fan
  digitalWrite(LED_PIN, LOW);
  digitalWrite(FAN_CTRL_PIN, LOW);

  ESP32Board::powerOff();
}

void TBeam1WBoard::setFanEnabled(bool enabled) {
  digitalWrite(FAN_CTRL_PIN, enabled ? HIGH : LOW);
}

bool TBeam1WBoard::isFanEnabled() const {
  return digitalRead(FAN_CTRL_PIN) == HIGH;
}
