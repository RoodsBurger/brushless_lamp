#pragma once

// Self-hosted signed OTA. A low-priority task polls a version manifest on GitHub
// Releases; when a newer signed image is published it downloads it (esp_https_ota
// verifies the RSA signature), then reboots into it once the lamp is idle.

void ota_start();
