# RP2040-FreeRTOS-Project
Raspberry pi pico RP2040 FreeRTOS Template Project

Based on : https://github.com/smittytone/RP2040-FreeRTOS

## Build Project
```
cmake -S . -B build/
cmake --build build
```

## Connect with picoprobe

```
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -s tcl
```

Flash :
```
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program PROJECT.elf verify reset exit"
```

## W5500

https://github.com/Wiznet/RP2040-HAT-FREERTOS-C

