################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
hal.obj: ../hal.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmspx --abi=eabi --data_model=restricted -O3 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/msp430_4.3.3/include" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/driverlib/MSP430F5xx_6xx" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/driverlib/MSP430F5xx_6xx/deprecated" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/USB_config" --advice:power="none" -g --define=__MSP430F5529__ --define=DEPRECATED --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="hal.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmspx --abi=eabi --data_model=restricted -O3 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/msp430_4.3.3/include" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/driverlib/MSP430F5xx_6xx" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/driverlib/MSP430F5xx_6xx/deprecated" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/USB_config" --advice:power="none" -g --define=__MSP430F5529__ --define=DEPRECATED --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

system_pre_init.obj: ../system_pre_init.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmspx --abi=eabi --data_model=restricted -O3 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/msp430_4.3.3/include" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/driverlib/MSP430F5xx_6xx" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/driverlib/MSP430F5xx_6xx/deprecated" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test" --include_path="C:/Users/Raghu Velagala/Documents/workspace_v6_0/lab8_test/USB_config" --advice:power="none" -g --define=__MSP430F5529__ --define=DEPRECATED --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="system_pre_init.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


