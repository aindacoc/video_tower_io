'''
Program title: Video Wall I/O
Description: Listens for serial messages from Arduino on COM8 and conditionally automates Extron MATRIX switcher program
Created by: Andres Indacochea
Created on : 4/27/23
Last modified by: Andres Indacochea
Last modified on: 5/3/23
'''
        ##IMPORT LIBRARIES
from pywinauto.application import Application # for interacting directly with MTRX.exe
from pywinauto.keyboard import send_keys       # for automating keystrokes
import time                                                                          # for waiting for stuff to load
import serial                                                                        # for listening to serial port


        ## SETUP
#Establish connection to Arduino
Arduino = serial.Serial('COM13')

VLC1 = Application(backend="uia").connect(process=24280)

VideoWindow1 = VLC1.window(handle=0xC0792)

EffectsWindow1=VLC1.window(handle=0xA406DC)

ColorsTab1 = EffectsWindow1["TabItem3"]

NegateColors1 = EffectsWindow1["CheckBox2"]

AdvancedTab1 = EffectsWindow1["TabItem6"]

#Establish connection to Extron MATRIX switcher program (already running)
SWITCHER = Application(backend="uia").connect(path="MTRX.exe")

#Define the main program window by window title
SWITCHERwindow = SWITCHER["Extron's MATRIX SWITCHERS Control Program    ver 8.4   ©1996-2014 Extron Electronics"]

#Define the GO button by element title
gobutton = SWITCHERwindow["GO"]

#Define the PRESETS combo box by automation id
presets = SWITCHERwindow.window(auto_id="23")

#Define the button that pulls down the menu by automation id
dropdown = presets.window(auto_id="DropDown")                                                 

#Confirm connections successful
print ("Serial and Extron MATRIX program connections successful, running  Video Wall I/O")

#Define combo list box handling method
def keyin_preset (keystroke):
    dropdown.click()                            #Click dropdown menu arrow
    SWITCHERwindow.wait("ready")         #Wait until list has loaded
    send_keys(keystroke)                  #Send intended keystroke

#Define preset selection method
def  select_preset (preset_number):  #Method name is select_preset
    if preset_number <5:                            #Need to handle 1-4 differently to other presets
        for x in range(preset_number):   #Repeat this part for the preset as necessary
            keyin_preset("{DOWN}")            #Use keyin_preset method to scroll the list
        gobutton.click()                                 #Once preset is selected, hit GO button
        
    elif (4 <  preset_number < 8):          #Handle 5-7 differently
        diff = 7 - preset_number                #Calculate number of keystrokes needed
        keyin_preset("{PGDN}")                #Send PAGE DOWN keystroke, scroll down 7 presets
        for x in range(diff):                           #Go up the list to get to 5, 6, or 7
            keyin_preset("{UP}")                   #Send UP keystroke instead
        gobutton.click()

    elif (7 <  preset_number < 12):      #Handle 8-11 differently
        diff = preset_number - 7
        keyin_preset("{PGDN}")                  
        for x in range(diff):                           
            keyin_preset("{DOWN}")                    
        gobutton.click()

    elif (11 <  preset_number < 15):    #Handle 12-14 differently
        diff = 14 - preset_number
        keyin_preset("{PGDN}")
        keyin_preset("{PGDN}")
        for x in range(diff):                           
            keyin_preset("{UP}")                      
        gobutton.click()

    elif (14<  preset_number < 17):     #Handle 15, 16 differently
        diff = preset_number - 14
        keyin_preset("{PGDN}")
        keyin_preset("{PGDN}")                   
        for x in range(diff):                         
            keyin_preset("{DOWN}")                  
        gobutton.click()
                                                                 

        ## MAIN PROGRAM
program_active = True           #Boolean factor that is always true

while program_active:           #Loop will always run since program_active = true
    Input = Arduino.read()      #WAIT for serial message from Arduino

    if (Input == b'1'):
        NegateColors1.click_input()
        print("Button 1 event detected")

    #What to do if serial message is '2'
    if (Input == b'2'):
        select_preset(2)
        print("Button 2 event detected")

        ## PROGRAM END, MUST BE MANUALLY STOPPED
