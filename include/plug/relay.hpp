// Hardware abstraction for the power relay and status LED.

#pragma once

#include "esp_err.h"

namespace app::plug {

class Relay {
public:
    Relay();
    ~Relay();

    esp_err_t init();
    void setState(bool state);
    bool getState() const;

private:
    bool m_state;
};

} // namespace app::plug
