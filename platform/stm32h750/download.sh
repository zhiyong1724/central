#!/bin/bash
openocd -f ./cmsis-dap.cfg -f ./stm32h7x.cfg -c "program build/bin/main.elf verify reset exit"