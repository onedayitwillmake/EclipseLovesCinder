################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../HelloEclipse.cpp 

OBJS += \
./HelloEclipse.o 

CPP_DEPS += \
./HelloEclipse.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/onedayitwillmake/GIT/LIBRARY/Cinder/boost -I/Users/onedayitwillmake/GIT/LIBRARY/Cinder/include -O0 -g -Wall -c -fmessage-length=0 -arch i386 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

