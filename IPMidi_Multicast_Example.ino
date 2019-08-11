/*
  (C) 2019 David J Bottrill, Shady Grove Electronics

  IPMidi Demo program to show how easy it is to send Midi events over UDP multicast
  For more information on IPMidi please see https://nerds.de/en/ipmidi.html
  For linux see QmidiNet https://qmidinet.sourceforge.io

  IPMidi has vely low latency and works well on SAMD21 and STM32 based boards
  This has been tested on the Adafruit Feather M0 and Generic STM32 BluePIll boards
  For STM32 this has been tested on STM32 Cores V1.61 https://github.com/stm32duino/Arduino_Core_STM32

  This program may not be styalistically perfect for two reasons:
  1)  I'm not an expert in C
  2)  This example has been cobbled together from code for PCBs I've designed that use I2C to link a master
      IPMidi scannner to slave scanners connected to Matrix and serial organ keyboards for use with
      Hauptwerk www.hauptwerk.com or GrandOrgue https://sourceforge.net/projects/ourorgan/
  This has been tested with the Adafruit Ethernet FeatherWing as well as a Generic W5500 Ethernet Shields
  easily found on Ebay.

  The program uses a circular buffer which is probably a bit overkill but it's a useful example and very
  easy to code.

  The program shows how to send single or multiple midi events in a single UDP package I'm not sure what this
  limit for how many midi events can be sent in a single UDP packet however it's at least 16
  you will need to adapt the code to send midi events that are not 3 bytes long but just pack them into
  sending buffer and QmidiNet or whatever you are using to receive IPMIdi should sort it out.

  Receiving IPMidi events is also simple but you will need code to unpack the buffer one event at a time
  taking into account how many bytes each midi command consists of.

  You also need to know that the channel variable here goes from 0 to 15 whilst some Midi devices and 
  software use the convention 1 to 16.

  The Demo tune is Bach's Little Fugue in G minor BWV578.

  (C) 2019 David J Bottrill, Shady Grove Electronics

  You are free to oopy, modify and improve, please seek my permission before using commercially.
  
*/
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 48                                   //increase UDP packet size default is 24

byte mac[] = {0x98, 0x76, 0xB6, 0x10, 0x85, 0x45};                  //This must be unique on your network
IPAddress multicast(225, 0, 0, 37);                                 //Multicast address for IPMidi
unsigned int destPort = 21928;                                      //21928 for IPMIDI port 0, 21929 for port 1 etc
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];                          //buffer to hold outgoing packet,

EthernetUDP Udp;                                                    //An EthernetUDP instance to let us send and receive packets over UDP

#define status_led 13
#define button 1                                                    //Input push button ground it to start playback

#include "fugue.h"                                                  //Demo tune in quasi midi format

byte chan = 1;                                                      //Midi channel to send on
byte velocity = 127;                                                //Maximum velocity

byte txbuf[3][256] = {};                                            //Output Circular buffer
byte inidx = 0;                                                     //Circular bufffer write position
byte outidx = 0;                                                    //Circular buffer read position
int bc = 0;                                                         //TX Midi buffer position

void setup() {

  pinMode(status_led, OUTPUT);
  digitalWrite(status_led, LOW);

  pinMode(button, INPUT_PULLUP);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) ;                                                 //wait for serial port to connect
  
  Serial.println("\nIPMidi Example\n");


  Ethernet.init(10);  // CS on most Arduino shields

  // start the Ethernet connection:
  Serial.println("Initialise Ethernet with DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected");
    }
    // no point in carrying on, so flash status LED forevermore:
    while (true) {
      digitalWrite(status_led, !digitalRead(status_led));
      delay(250);
    }
  }


  // start UDP Multicast
  Serial.println("Starting IPMidi.");
  Udp.beginMulticast(multicast, destPort);

  Serial.print("\nIP address : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Multicast address : ");
  Serial.print(multicast);
  Serial.print("  Port : ");
  Serial.println(destPort);
  Serial.print("TX Buffer : ");
  Serial.println(UDP_TX_PACKET_MAX_SIZE);
  Serial.println("\nConnect pin 1 to ground to start playing\n");

}

void loop() {

  if (digitalRead(button) == LOW) playTune();

  delay(1);

}


//Function to add data to send buffer
void bufferWrite(byte par1, byte par2, byte par3) {
  txbuf[0][inidx] = par1;                                           //Write to circular buffer
  txbuf[1][inidx] = par2;                                           //Write to circular buffer
  txbuf[2][inidx] = par3;                                           //Write to circular buffer
  inidx++;                                                          //Increment buffer input index
}

void playTune(void) {
  Serial.print("Playing Demo Tune...");
  int i = 0;
  int count = 0 ;
  unsigned long l_time;
  while (i < t_count) {
    l_time = micros();
    //Play all events at current timecode
    while (t_time[i] == count) {                                    //Is there an event?
      if (t_onoff[i] == 1) {                                        //Note on event
        bufferWrite(0x90 + chan, t_note[i], velocity);
      } else {                                                      //Note off event
        bufferWrite(0x80 + chan, t_note[i], velocity);
      }
      i++;                                                          //Inc event counter
    }                                                               //any more events?
    sendMidi();                                                     //Send midi events
    while (micros() < l_time + t_delay);                            //Midi playbock time melay
    count++;
  }
  Serial.println("Done\n");
}


void sendMidi(void) {
  //Sends multiple Midi messages in a single UDP packet
  if (inidx != outidx) {                                            //If read and write pointers are different
    bc = 0;                                                         //then there must be data in the buffer
    while (inidx != outidx && bc < (UDP_TX_PACKET_MAX_SIZE - 3)) {  //Put all pending Midi event in transmit buffer
      packetBuffer[bc] = txbuf[0][outidx];                          //First Midi byte
      bc++;
      packetBuffer[bc] = txbuf[1][outidx];                          //Second Midi byte
      bc++;
      packetBuffer[bc] = txbuf[2][outidx];                          //Third Mid byte
      bc++;
      outidx++;
    }
    Udp.beginPacket(multicast, destPort);                           //Start UDP packey send
    Udp.write(packetBuffer, bc);                                    //Send buffer
    Udp.endPacket();                                                //Finish sending    
  }
}
