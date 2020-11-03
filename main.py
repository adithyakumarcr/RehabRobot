#Libraries that we need to import before we start a program. 
import os
import serial
import struct
import csv
import os
import time
import keyboard
import matplotlib.pyplot as plot 
import numpy as np

#If you find no module found, please install the module using pip installer. 

#Arduino port refers to the USB port connected to our PC. It may vary from PC to PC. It is always a good practice to check for PORTID from Ardiuno Application
arduino_port = "COM5"

baud= 9600
samples=200
print_labels=True

impedances= []
position=[]

#Connecting the ardiuno port to this particular program
ser= serial.Serial(arduino_port, baud)
print("Connected to Arduino port:" +arduino_port)



#To get a put character
if os.name == 'nt':
    import msvcrt
    def getch():
        return msvcrt.getch().decode()
else:
    import sys, tty, termios
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    def getch():
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

#Default position control program to control Dyamixel Motors

from dynamixel_sdk import *                    # Uses Dynamixel SDK library

# Control table address
ADDR_PRO_TORQUE_ENABLE      = 64               # Control table address is different in Dynamixel model
ADDR_PRO_GOAL_POSITION      = 116
ADDR_PRO_PRESENT_POSITION   = 132

# Protocol version
PROTOCOL_VERSION            = 2.0               # See which protocol version is used in the Dynamixel

# Default setting
DXL_ID                      = 2                 # Dynamixel ID : 1
BAUDRATE                    = 1000000             # Dynamixel default baudrate : 57600
DEVICENAME                  = 'COM10'    # Check which port is being used on your controller
                                                # ex) Windows: "COM1"   Linux: "/dev/ttyUSB0" Mac: "/dev/tty.usbserial-*"

TORQUE_ENABLE               = 1                 # Value for enabling the torque
TORQUE_DISABLE              = 0                 # Value for disabling the torque
DXL_MINIMUM_POSITION_VALUE  = 2382           # Dynamixel will rotate between this value
DXL_MAXIMUM_POSITION_VALUE  = 3647            # and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)
DXL_MOVING_STATUS_THRESHOLD = 20                # Dynamixel moving status threshold

index = 0
dxl_goal_position = [DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE]         # Goal position

# Initialize PortHandler instance
# Set the port path
# Get methods and members of PortHandlerLinux or PortHandlerWindows
portHandler = PortHandler(DEVICENAME)

# Initialize PacketHandler instance
# Set the protocol version
# Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
packetHandler = PacketHandler(PROTOCOL_VERSION)

# Open port
if portHandler.openPort():
    print("Succeeded to open the port")
else:
    print("Failed to open the port")
    print("Press any key to terminate...")
    getch()
    quit()


# Set port baudrate
if portHandler.setBaudRate(BAUDRATE):
    print("Succeeded to change the baudrate")
else:
    print("Failed to change the baudrate")
    print("Press any key to terminate...")
    getch()
    quit()

# Enable Dynamixel Torque
dxl_comm_result, dxl_error = packetHandler.write1ByteTxRx(portHandler, DXL_ID, ADDR_PRO_TORQUE_ENABLE, TORQUE_ENABLE)
if dxl_comm_result != COMM_SUCCESS:
    print("%s" % packetHandler.getTxRxResult(dxl_comm_result))
elif dxl_error != 0:
    print("%s" % packetHandler.getRxPacketError(dxl_error))
else:
    print("Dynamixel has been successfully connected")


#Start of a while loop, that means the program keeps running until we force stop it. 

while 1:
    #Simple UI to select options.
    keypress=input("1. Calibrate \n2.Measure\n3.Stop \n Enter Options:")
    
    if keypress=="1":
        n=2                 #In case of calibration the motors moves from Current Postion -> Goal Positon (Extreme End_Right) -> Goal_Position (Extreme_left)
    elif keypress=="2" :
        n=10                #Measure by default runs 5times both the sides, thats why the n is set to 10. The motor rotates back and forth between two Goal Positions.
    elif keypress=="3" :
        break               #Break- Breaks the while loop
    else:
        print("Enter valid input")
        n=0

    i=0

    #The Nested while loop is used to move the motor to the both ends. 
    while i<n:  #Until the condition is satisfed, the end effectors keep moving in both the directions. i is set to 0 and n value is recieved from the above if-else condition. 
        # Write goal position
        dxl_comm_result, dxl_error = packetHandler.write4ByteTxRx(portHandler, DXL_ID, ADDR_PRO_GOAL_POSITION, dxl_goal_position[index])
        if dxl_comm_result != COMM_SUCCESS:
            print("%s" % packetHandler.getTxRxResult(dxl_comm_result))
        elif dxl_error != 0:
            print("%s" % packetHandler.getRxPacketError(dxl_error))
    
        i=i+1
        while 1:
            print("%s" % packetHandler.getRxPacketError(dxl_error))
            # Read present position

            #The below function gives data of the present position. 
            dxl_present_position, dxl_comm_result, dxl_error = packetHandler.read4ByteTxRx(portHandler, DXL_ID, ADDR_PRO_PRESENT_POSITION)
            if dxl_comm_result != COMM_SUCCESS:
                print("%s" % packetHandler.getTxRxResult(dxl_comm_result))
            elif dxl_error != 0:
                print("%s" % packetHandler.getRxPacketError(dxl_error))

            print("[ID:%03d] GoalPos:%03d  PresPos:%03d" % (DXL_ID, dxl_goal_position[index], dxl_present_position))

            if not abs(dxl_goal_position[index] - dxl_present_position) > DXL_MOVING_STATUS_THRESHOLD:
                break
            
            #At this point, The program reads the line from Ardiuno serial monitor
            getData= str(ser.readline(), 'utf-8')
            data=getData[0:][:-1]

            #Condition to check is the first 10 letters start with Impedance.
            if data[:10]=="Impedance:":

                #impedance values is something after 10 letters
                impedance=data[10:]
                #Convert string to float
                impedance=float(impedance)/1000
                #Print the values of impedance
                print(impedance)
                #Appended to Array/List  To understand, what list is CHECK: https://www.w3schools.com/python/python_lists.asp
                impedances.append(impedance)
                
                #Converting the position to Angle
                angle=dxl_present_position/11.37647058823529
                print(angle)
                #Appending the angle to data to an List
                position.append(angle)

                if impedance!=0:
                    #Writing data to a csv file for references in the future
                    with open("Impedance_values.csv", "w") as csv_file:
                        writer = csv.writer(csv_file, delimiter= ';')
                        writer.writerow((impedance,dxl_present_position))

        # Change goal position
        #Ones the goal position is reached, we need to switch sides. 
        if index == 0:
            index = 1
        else:
            index = 0


print("Data collection complete!")


# Disable Dynamixel Torque (Disconnect Motor from the program)
dxl_comm_result, dxl_error = packetHandler.write1ByteTxRx(portHandler, DXL_ID, ADDR_PRO_TORQUE_ENABLE, TORQUE_DISABLE)
if dxl_comm_result != COMM_SUCCESS:
    print("%s" % packetHandler.getTxRxResult(dxl_comm_result))
elif dxl_error != 0:
    print("%s" % packetHandler.getRxPacketError(dxl_error))

# Close Ardiuno ports
portHandler.closePort()

#Plot a graph between Position (X-Axis) vs Impedance(Y-axis)
plot.scatter(position, impedances)
plot.title('Impedance vs position')
plot.xlabel('position')
plot.ylabel('Impedance (KOhms)')
plot.show()
