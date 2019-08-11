# IPMidi_example
IPMid implementation for Arduino using Multicast UDP

 IPMidi Demo program to show how easy it is to send Midi events over UDP multicast
  For more information on IPMidi please see https://nerds.de/en/ipmidi.html
  For linux see QmidiNet https://qmidinet.sourceforge.io

  IPMidi has very low latency and works well on SAMD21 and STM32 based boards
  This has been tested on the Adafruit Feather M0 and Generic STM32 BluePIll boards
  For STM32 this has been tested on STM32 Cores V1.61 https://github.com/stm32duino/Arduino_Core_STM32

  This program may not be stylistically perfect for two reasons:
  1)  I'm not an expert in C
  2)  This example has been cobbled together from code for PCBs I've designed that use I2C to link a master
      IPMidi scannner to slave scanners connected to Matrix and serial organ keyboards for use with
      Hauptwerk www.hauptwerk.com or GrandOrgue https://sourceforge.net/projects/ourorgan/
  This has been tested with the Adafruit Ethernet FeatherWing as well as a Generic W5500 Ethernet Shields
  easily found on Ebay.

  The program uses a circular buffer which is probably a bit overkill but it's a useful example and very
  easy to code.

  The program shows how to send single or multiple midi events in a single UDP package I'm not sure what the
  limit is for how many midi events can be sent in a single UDP packet however it's at least 16
  you will need to adapt the code to send midi events that are not 3 bytes long but just pack them into
  sending buffer and QmidiNet or whatever you are using to receive IPMIdi should sort it out.

  Receiving IPMidi events is also simple but you will need code to unpack the buffer one event at a time
  taking into account how many bytes each midi command consists of.

  You also need to know that the channel variable here goes from 0 to 15 whilst some Midi devices and 
  software use the convention 1 to 16.

  The Demo tune is Bach's Little Fugue in G minor BWV578.

  (C) 2019 David J Bottrill, Shady Grove Electronics

  You are free to oopy, modify and improve, please seek my permission before using commercially.
