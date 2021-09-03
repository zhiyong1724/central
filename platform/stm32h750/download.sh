#!/bin/bash
openocd -f ./cmsis-dap.cfg -f ./stm32h7x.cfg -c "program build/bin/flash.bin verify reset exit 0x8000000"