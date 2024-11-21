#ifndef PI_HAL_WIRINGPI_H
#define PI_HAL_WIRINGPI_H

// include RadioLib
#include <RadioLib.h>

// include the library for Raspberry GPIO pins
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <cerrno>
#include <sched.h>
#include <inttypes.h>


#define PI_RISING         (INT_EDGE_RISING)
#define PI_FALLING        (INT_EDGE_FALLING)
#define PI_INPUT          (INPUT)
#define PI_OUTPUT         (OUTPUT)
#define PI_MAX_USER_GPIO  (31)

// forward declaration of alert handler that will be used to emulate interrupts
static void wiringPiInterruptHandler(void);

// create a new Raspberry Pi hardware abstraction layer
// using the wiringPi library
// the HAL must inherit from the base RadioLibHal class
// and implement all of its virtual methods
class PiHal : public RadioLibHal {
  public:
    // default constructor - initializes the base HAL and any needed private members
    PiHal(uint8_t spiChannel, uint32_t spiSpeed = 2000000)
      : RadioLibHal(PI_INPUT, PI_OUTPUT, LOW, HIGH, PI_RISING, PI_FALLING),
      _spiChannel(spiChannel),
      _spiSpeed(spiSpeed) {
    }

    void init() override {
      // Initialize wiringPi library
      if (wiringPiSetup() == -1) {
        fprintf(stderr, "Failed to initialize wiringPi\n");
        return;
      }

      // now the SPI
      spiBegin();
    }

    void term() override {
      // stop the SPI
      spiEnd();
    }

    // GPIO-related methods (pinMode, digitalWrite etc.) should check
    // RADIOLIB_NC as an alias for non-connected pins
    void pinMode(uint32_t pin, uint32_t mode) override {
      if(pin == RADIOLIB_NC) {
        return;
      }

      ::pinMode(pin, mode);
    }

    void digitalWrite(uint32_t pin, uint32_t value) override {
      if(pin == RADIOLIB_NC) {
        return;
      }

      ::digitalWrite(pin, value);
    }

    uint32_t digitalRead(uint32_t pin) override {
      if(pin == RADIOLIB_NC) {
        return(0);
      }

      return ::digitalRead(pin);
    }

    void attachInterrupt(uint32_t interruptNum, void (*interruptCb)(void), uint32_t mode) override {
      if(interruptNum == RADIOLIB_NC || interruptNum > PI_MAX_USER_GPIO) {
        return;
      }

      // enable emulated interrupt
      interruptEnabled[interruptNum] = true;
      interruptModes[interruptNum] = mode;
      interruptCallbacks[interruptNum] = interruptCb;

      // set wiringPi interrupt callback
      if (wiringPiISR(interruptNum, mode, interruptCb) < 0) {
        fprintf(stderr, "Could not set ISR for pin %" PRIu32 "\n", interruptNum);
      }
    }

    void detachInterrupt(uint32_t interruptNum) override {
      if(interruptNum == RADIOLIB_NC || interruptNum > PI_MAX_USER_GPIO) {
        return;
      }

      // clear emulated interrupt
      interruptEnabled[interruptNum] = false;
      interruptModes[interruptNum] = 0;
      interruptCallbacks[interruptNum] = NULL;
      
      // No direct way to detach in wiringPi, disable callback
      if (wiringPiISR(interruptNum, INT_EDGE_SETUP, nullptr) < 0) {
        fprintf(stderr, "Could not detach ISR for pin %" PRIu32 "\n", interruptNum);
      }
    }

    void delay(unsigned long ms) override {
      if(ms == 0) {
        sched_yield();
        return;
      }

      ::delay(ms);
    }

    void delayMicroseconds(unsigned long us) override {
      if(us == 0) {
        sched_yield();
        return;
      }

      ::delayMicroseconds(us);
    }

    void yield() override {
      sched_yield();
    }

    unsigned long millis() override {
      return ::millis();
    }

    unsigned long micros() override {
      return ::micros();
    }

    long pulseIn(uint32_t pin, uint32_t state, unsigned long timeout) override {
      if(pin == RADIOLIB_NC) {
        return(0);
      }

      this->pinMode(pin, PI_INPUT);
      uint32_t start = this->micros();
      uint32_t curtick = this->micros();

      while(this->digitalRead(pin) == state) {
        if((this->micros() - curtick) > timeout) {
          return(0);
        }
      }

      return(this->micros() - start);
    }

    void spiBegin() {
      // SPI initialization is done in init
      if (wiringPiSPISetup(_spiChannel, _spiSpeed) < 0) {
        fprintf(stderr, "Could not open SPI handle: %s\n", strerror(errno));
      }
    }

    void spiBeginTransaction() {}

    void spiTransfer(uint8_t* out, size_t len, uint8_t* in) {
      uint8_t* buffer = new uint8_t[len];
      memcpy(buffer, out, len);

      if (wiringPiSPIDataRW(_spiChannel, buffer, len) == -1) {
        fprintf(stderr, "Could not perform SPI transfer: %s\n", strerror(errno));
      }

      memcpy(in, buffer, len);
      delete[] buffer;
    }

    void spiEndTransaction() {}

    void spiEnd() {
      // No specific end method for SPI in wiringPi
    }

    void tone(uint32_t pin, unsigned int frequency, unsigned long duration = 0) {
      // No direct implementation in wiringPi, can be customized if needed
    }

    void noTone(uint32_t pin) {
      // No direct implementation in wiringPi, can be customized if needed
    }

    // interrupt emulation
    bool interruptEnabled[PI_MAX_USER_GPIO + 1];
    uint32_t interruptModes[PI_MAX_USER_GPIO + 1];
    typedef void (*RadioLibISR)(void);
    RadioLibISR interruptCallbacks[PI_MAX_USER_GPIO + 1];

  private:
    // the HAL can contain any additional private members
    const unsigned int _spiSpeed;
    const uint8_t _spiChannel;
};

#endif