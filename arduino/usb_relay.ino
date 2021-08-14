#include <EEPROM.h>

#define CIRCUIT_CONF_ADDR 0
#define DEV_NAME_MAX      16

void query_circuit_conf(char* tx_buf)
{
  int circuit_conf = EEPROM.read(0);
  if ( circuit_conf == 255 )
  {
    sprintf(tx_buf, "#CC?");
  }
  else
  {
    sprintf(tx_buf, "#CC%d",circuit_conf);
  }
}

void query_dev_name(char* tx_buf)
{
  char dev_name[DEV_NAME_MAX];
  int i;
  
  for ( i = 0; i < DEV_NAME_MAX; i++ )
  {
    // read from offset 1
    dev_name[i] = EEPROM.read(1+i);
    if ( dev_name[i] == -1 )
      dev_name[i] = '\0';
    if ( dev_name[i] == '\0' )
      break;
  }
  dev_name[DEV_NAME_MAX-1] = '\0';
  
  sprintf(tx_buf, "#DN!%s",dev_name);
}

void update_dev_name(char* dev_name)
{
  int i;
  for ( i = 0; i < DEV_NAME_MAX; i++ )
  {
    // read from offset 1
    EEPROM.update(1+i, dev_name[i]);
    if ( dev_name[i] == '\0' )
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(3, OUTPUT); 
  digitalWrite(3, HIGH);
  Serial.setTimeout(200);
}

void loop() {
  // put your main code here, to run repeatedly:
  size_t len;
  char rx_buf[32];
  char tx_buf[32];
  
  //while(Serial.available())
  {
    len = Serial.readBytes(rx_buf, sizeof(rx_buf));

    if ( len > 0 )
    {
      if ( !memcmp("@SW1", rx_buf, 4) ) 
      {
        // relay on
        digitalWrite(3, LOW); 
      }
      else if ( !memcmp("@SW0", rx_buf, 4) )
      {
        // relay off
        digitalWrite(3, HIGH);
      }
      else if ( !memcmp("@SW?", rx_buf, 4) )
      {
        // query current status
        int status = digitalRead(3);
        sprintf(tx_buf, "#SW%d", !status);
        Serial.write(tx_buf);
      }
      else if( !memcmp("@CC", rx_buf, 3) )
      {
        if ( rx_buf[3] == '?' )
        {
          // query circuit config
          query_circuit_conf(tx_buf);
          Serial.write(tx_buf);
        }
        else if ( rx_buf[3] == '0' )
        {
          // store normally open
          EEPROM.update(CIRCUIT_CONF_ADDR, 0);
          query_circuit_conf(tx_buf);
          Serial.write(tx_buf);
        }
        else if ( rx_buf[3] == '1' )
        {
          // store normally close
          EEPROM.update(CIRCUIT_CONF_ADDR, 1);
          query_circuit_conf(tx_buf);
          Serial.write(tx_buf);
        }
      }
      else if( !memcmp("@DN?", rx_buf, 4) )
      {
        //query device name
        query_dev_name(tx_buf);
        Serial.write(tx_buf);
      }
      else if ( !memcmp("@DN!", rx_buf, 4) )
      {
        char dev_name[DEV_NAME_MAX];

        if ( (len-4) >= DEV_NAME_MAX )
        {
          memcpy(dev_name, &rx_buf[4], DEV_NAME_MAX-1);
          dev_name[DEV_NAME_MAX-1] = '\0';
        }
        else
        {
          memcpy(dev_name, &rx_buf[4], len-4);
          dev_name[len-4] = '\0';  
        }
 
        if (dev_name[0] != '\0' )
        {
          update_dev_name(dev_name);
        }
      }
    }
  }
}
