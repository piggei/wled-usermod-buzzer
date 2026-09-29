#include "WLEDBuzzerService.h"

namespace {
WLEDBuzzerService* serviceInstance = nullptr;
}

WLEDBuzzerService* WLEDBuzzerService::instance() {
  return serviceInstance;
}

void WLEDBuzzerService::setInstance(WLEDBuzzerService* instance) {
  serviceInstance = instance;
}
