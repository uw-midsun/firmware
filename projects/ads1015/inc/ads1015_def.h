#define ADS1015_I2C_ADDRESS_GND ((uint8_t)0x48)

#define ADS1015_ADDRESS_POINTER_CONV        ((uint8_t)0x0)
#define ADS1015_ADDRESS_POINTER_CONFIG      ((uint8_t)0x1)
#define ADS1015_ADDRESS_POINTER_LO_THRESH   ((uint8_t)0x2)
#define ADS1015_ADDRESS_POINTER_HI_THRESH   ((uint8_t)0x3)

#define ADS1015_LO_THRESH_REGISTER_MSB ((uint8_t)0x0)
#define ADS1015_HI_THRESH_REGISTER_MSB ((uint8_t)0xFF)

//typedef enum {
#define  ADS1015_NO_EFFECT          ((uint8_t)0x0 << 7)   
#define  ADS1015_START_SINGLE_CONV  ((uint8_t)0x1 << 7)   
// ADS1015_OS_CONVERTING   = 0,     
// ADS1015_OS_NOT_CONVERTING  ,
//  NUM_ADS1015_OSS,
// } ADS1015OS;

// typedef enum {
#define  ADS1015_AIN_0         ((uint8_t)0x4 << 4)
#define  ADS1015_AIN_1         ((uint8_t)0x5 << 4)
#define  ADS1015_AIN_2         ((uint8_t)0x6 << 4) 
#define  ADS1015_AIN_3         ((uint8_t)0x7 << 4)
//  NUM_ADS1015_AINS,
// } ADS1015MUX;

//typedef enum {
#define  ADS1015_PGA_FSR_0        ((uint8_t)0x0 << 1)  // ±6.144 V
#define  ADS1015_PGA_FSR_1        ((uint8_t)0x1 << 1)  // ±4.096 V
#define  ADS1015_PGA_FSR_DEFAULT  ((uint8_t)0x2 << 1)  // ±2.048 V
#define  ADS1015_PGA_FSR_3        ((uint8_t)0x3 << 1)  // ±1.024 V 
#define  ADS1015_PGA_FSR_4        ((uint8_t)0x4 << 1)  // ±0.512 V
#define  ADS1015_PGA_FSR_5        ((uint8_t)0x5 << 1)  // ±0.256 V
//  NUM_ADS1015_PGA_FSRS,
//} ADS1015PGA;

// typedef enum {
#define  ADS1015_CONVERSION_MODE_CONT     ((uint8_t)0x0)
#define  ADS1015_CONVERSION_MODE_SINGLE   ((uint8_t)0x1) // default
//  NUM_ADS1015_CONVERSION_MODES,
//} ADS1015ConversionMode;

// typedef enum {
#define  ADS1015_DATA_RATE_0        ((uint8_t)0x0 << 5)   // 128 SPS
#define  ADS1015_DATA_RATE_1        ((uint8_t)0x1 << 5)   // 250 SPS
#define  ADS1015_DATA_RATE_2        ((uint8_t)0x2 << 5)   // 490 SPS
#define  ADS1015_DATA_RATE_3        ((uint8_t)0x3 << 5)   // 920 SPS
#define  ADS1015_DATA_RATE_DEFAULT  ((uint8_t)0x4 << 5)   // 1600 SPS
#define  ADS1015_DATA_RATE_5        ((uint8_t)0x5 << 5)   // 2400 SPS
#define  ADS1015_DATA_RATE_6        ((uint8_t)0x6 << 5)   // 3300 SPS
//  NUM_ADS1015_DATA_RATES,
// } ADS1015DataRate;

//typedef enum {
#define  ADS1015_COMP_MODE_TRAD        ((uint8_t)0x0 << 4)
#define  ADS1015_COMP_MODE_WINDOW      ((uint8_t)0x1 << 4) 
//  NUM_ADS1015_COMP_MODES,   
//} ADS1015CompMode;

//typedef enum {
#define  ADS1015_COMP_POL_LOW      ((uint8_t)0x0 << 3)
#define  ADS1015_COMP_POL_HIGH     ((uint8_t)0x1 << 3)
//  NUM_ADS1015_COMP_POLS,
//} ADS1015CompPol;

//typedef enum {
#define  ADS1015_COMP_LAT_NON_LATCHING ((uint8_t)0x0 << 2)
#define  ADS1015_COMP_LAT_LATCHING     ((uint8_t)0x1 << 2)
//  NUM_ADS1015_COMP_LATS,
//} ADS1015CompLat;

//typedef enum {
#define  ADS1015_COMP_QUE_1_CONV        ((uint8_t)0x0) 
#define  ADS1015_COMP_QUE_2_CONV        ((uint8_t)0x1)
#define  ADS1015_COMP_QUE_4_CONV        ((uint8_t)0x2)
#define  ADS1015_COMP_QUE_DISABLE_COMP  ((uint8_t)0x3)
// NUM_ADS1015_COMP_QUES
//} ADS1015CompQue;

#define ADS1015_I2C_GENERAL_CALL ((uint8_t)0x0)

#define ADS1015_RESET_BYTE  ((uint8_t)0x6)

#define CONFIG_REGISTER_MSB_0                                            \
  (ADS1015_START_SINGLE_CONV | ADS1015_AIN_0 | ADS1015_PGA_FSR_DEFAULT | \
   ADS1015_CONVERSION_MODE_CONT)

#define CONFIG_REGISTER_MSB_1                                            \
  (ADS1015_START_SINGLE_CONV | ADS1015_AIN_1 | ADS1015_PGA_FSR_DEFAULT | \
   ADS1015_CONVERSION_MODE_CONT)

#define CONFIG_REGISTER_MSB_2                                            \
  (ADS1015_START_SINGLE_CONV | ADS1015_AIN_2 | ADS1015_PGA_FSR_DEFAULT | \
   ADS1015_CONVERSION_MODE_CONT)

#define CONFIG_REGISTER_MSB_3                                            \
  (ADS1015_START_SINGLE_CONV | ADS1015_AIN_3 | ADS1015_PGA_FSR_DEFAULT | \
   ADS1015_CONVERSION_MODE_CONT)

#define CONFIG_REGISTER_LSB                                                     \
  (ADS1015_DATA_RATE_DEFAULT | ADS1015_COMP_MODE_TRAD | ADS1015_COMP_POL_HIGH | \
   ADS1015_COMP_LAT_NON_LATCHING | ADS1015_COMP_QUE_1_CONV)
