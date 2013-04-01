
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: pTop

# Tool invocations
pTop: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o"pTop" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES)
	@echo 'Finished clean: $@'

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) pTop
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
