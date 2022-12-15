
#ifndef CONFIGH
#define CONFIGH


#define EZVIZ_T30  0
#define EIGHT_TREE 0
#define ESP_01     1

#if  GLOBE_PLUG
  #define   LED      4
  #define   RELAY   12
  #define   BUTTON  14
#endif 


#if  EZVIZ_T30
  #define   LED    4
  #define   RELAY  12
  #define   BUTTON 14
#endif 

#if  EIGHT_TREE
  #define   LED    23
  #define   RELAY  12
  #define   BUTTON 14
#endif 

#if  ESP_01
  #define   LED    1
  #define   RELAY  0
  #define   BUTTON 0
#endif 


#endif
