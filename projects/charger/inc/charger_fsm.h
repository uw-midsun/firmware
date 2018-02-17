#pragma once

// Charger design:
//
// Charger Disconnected:
// - Do nothing
//
// Charger Connected:
// - Request permission periodically
//
// Charger Charging:
// - Request permission periodically
// - Charge while permission is granted and periodically publish data
