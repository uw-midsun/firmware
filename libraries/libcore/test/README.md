# Warning: libcore tests need to reside in ms-common/test!

This is because the Reset Handler symbol definition for the stm32f0xx
resides in the stm32f0xx library and must be linked in prior to building any 
tests in order to initialize the hardware. As a result, libcore would have to 
circularly depend on stm32f0xx if this library had a tests folder. To avoid
this and other issues any tests for code in this folder will be in ms-common
instead.
