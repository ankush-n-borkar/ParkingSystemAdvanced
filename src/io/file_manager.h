#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "../core/parking_system.h"

/* Save parking state to file */
int save_parking_state(ParkingSystem *ps, const char *filename);

/* Load parking state from file */
int load_parking_state(ParkingSystem *ps, const char *filename);

/* Log transaction */
void log_transaction(const char *transaction_type, const char *slot_id, const char *vehicle_number);

#endif
