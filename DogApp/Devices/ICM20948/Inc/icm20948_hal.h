#ifndef ICM_20948_HAL_H
#define ICM_20948_HAL_H


#include "Icm20948.h"
#include "Devices/Drivers/Ak0991x/Ak0991x.h"
#include "Devices/SensorTypes.h"
#include "Devices/SensorConfig.h"
#include "EmbUtils/InvScheduler.h"
#include "EmbUtils/RingByteBuffer.h"
#include "EmbUtils/Message.h"
#include "EmbUtils/ErrorHelper.h"
#include "EmbUtils/dataconverter.h"
#include "EmbUtils/RingBuffer.h"
// #include "DynamicProtocol/DynProtocol.h"
// #include "DynamicProtocol/DynProtocolTransportUart.h"
#include "Icm20948MPUFifoControl.h"
#include "Icm20948Dmp3Driver.h"

#include "inv_sensor.h"

void inv_sys_initial_dmp(void);




#endif