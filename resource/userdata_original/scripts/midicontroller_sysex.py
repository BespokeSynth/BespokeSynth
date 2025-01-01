#requires:
# - a midicontroller named "midicontroller"
# - the send_sysex to be active in the triangle menu of the midicontroller, in order to receive on_sysex events
# - a MIDI device able to send sysex data

import midicontroller

controller = midicontroller.get("midicontroller")
controller.add_script_listener(me.me())

#Customize this to match the device sysex message
vendorID = 0x01
deviceID = 0x05
sysexDataLen = 35

def restore_bytes_from_nibbles(nibbles):
    if len(nibbles) % 2 != 0:
        raise ValueError("Expecting an even number of nibbles")
    
    bytes_data = []
    for i in range(0, len(nibbles), 2):
        byte = (nibbles[i + 1] << 4) | nibbles[i]
        bytes_data.append(byte)
    return bytes_data

def extract_bits(byte, start, length):
    mask = (1 << length) - 1
    return (byte >> start) & mask

def on_sysex(data):
    #Calculate the length of the received data
    data_length = len(data)
    me.output(f"length: {data_length}")
    
    #python will display regular characters in a hex string
    #Convert bytes to a string of all hexadecimal values 
    hex_output = ' '.join(f'{byte:02X}' for byte in bytes(data)) 
    me.output(f"Hex output: {hex_output}")
    
    if data_length < 2:
        me.output(f"Sysex data length invalid: {data_length}")
        return   
    if data[0] != vendorID:             #Check Vendor ID
        me.output(f"Sysex vendor ID not equal to expected vendor ID ({vendorID}): {data[0]}")
        return
    if data[1] != deviceID:             #Check Device ID
        me.output(f"Sysex device ID not equal to expected device ID ({deviceID}): {data[1]}")
        return
    
    #Parsing of this vendor specific sysex starts here
    
    if data_length != sysexDataLen:     #Check sysex data length
        me.output(f"Sysex data length invalid ({sysexDataLen}): {data_length}")
        return

    # in this sysex example, data[2] contains the patch_number
    patch_number =  data[2:]

    #sysex data is send as nibbles in bytes
    #parse sysex data here, or convert to bytes first:      
    nibbles = data[3:]
    bytes_data = restore_bytes_from_nibbles(nibbles)
    
    #Debug: output bytes in binary format
    bit_str = ''
    for byte in bytes_data:
        bit_str = bit_str + f" {byte:08b}"
    me.output(bit_str)
    
    #Example: Extract stuffed data from bytes
    #Byte
    #0:   B1 B0 .. .. .. .. .. ..
    #1:   .. .. .. .. .. B4 B3 B2
    
    valueB = extract_bits(bytes_data[0], 6, 2) | (extract_bits(bytes_data[1], 0, 3) << 2)
    me.output(f"valueB: {valueB}")

#Example of sysex data
data = [0x01, 0x05, 0x00, 0x08, 0x08, 0x0, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0x09, 0x00, 0x00, 0x06, 0x06, 0x07, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x08, 0x07, 0x04, 0x01, 0x04, 0x02]  # Sample data
#Manually call function to trigger example
on_sysex(data)

#on_midi events will be received as well, so this has to be defined as well
def on_midi(messageType, control, value, channel):
   debug = str(messageType) + " " + str(control) + " " + str(value) + " " + str(channel)
   me.output(debug)