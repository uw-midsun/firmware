#include "calib.h"

StatusCode calib_init(CalibStorage *calib) {
  status_ok_or_return(
      persist_init(&calib->persist, CALIB_FLASH_PAGE, &calib->blob, sizeof(calib->blob)));

  return persist_ctrl_periodic(&calib->persist, false);
}

StatusCode calib_commit(CalibStorage *calib) {
  return persist_commit(&calib->persist);
}

CalibBlob *calib_blob(CalibStorage *calib) {
  return &calib->blob;
}
