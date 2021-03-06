typedef unsigned char                 *PUINT8;
typedef unsigned char volatile          UINT8V;

#include "ch554.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>
#include <ch554_usb.h>
#include <bootloader.h>

__xdata uint8_t readFlag = 0;	
__xdata char sPath[] = "Dies ist der CH552 Beispiel Text. Druecke Num um die LED yu schalten.";							
__xdata char *pStr = sPath;
uint32_t millis, last,last1;

#define LED_PIN1 0
SBIT(LED1, 0xB0, LED_PIN1);
#define LED_PIN2 1
SBIT(LED2, 0xB0, LED_PIN2);

SBIT(Ep2InKey, 0xB0, 2);

#define THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE

__xdata __at (0x0000) uint8_t  Ep0Buffer[DEFAULT_ENDP0_SIZE];	  
__xdata __at (0x0050) uint8_t  Ep1Buffer[DEFAULT_ENDP1_SIZE];   
__xdata __at (0x000a) uint8_t  Ep2Buffer[2*MAX_PACKET_SIZE];	 

uint8_t   SetupReq,SetupLen,Ready,Count,FLAG,UsbConfig;
PUINT8  pDescr;                                                             
USB_SETUP_REQ   SetupReqBuf;    


uint8_t a,b,numlock;

void jump_to_bootloader()
{
	USB_INT_EN = 0;
	USB_CTRL = 0x06;
	EA = 0;
	mDelaymS(100);
	bootloader();
	while(1);
}

void	mTimer0Interrupt( void ) __interrupt (INT_NO_TMR0)
{                                              
	TH0 = (65536 - 2000)/256;  // Reload
	TL0 = (65536 - 2000)%256;  // Reload    
	millis++;
}

#define		BIT0		(0X01)
#define		BIT1		(0X02)
#define		BIT2		(0X04)
#define		BIT3		(0X08)
#define		BIT4		(0X10)
#define		BIT5		(0X20)
#define		BIT6		(0X40)
#define		BIT7		(0X80)

/* Macro define */
#define		CHX				(0X00)				
#define		CH2				(BIT2)
#define		CH3				(BIT3)
#define		CH_FREE			(0x07)				

#define		TH_VALUE		(100)
#define		TOUCH_NUM		(0x04)
#define		SAMPLE_TIMES	(0x05)

__xdata uint8_t 	TK_Code[TOUCH_NUM] = {							
	0x03, 0x04, 										
};		

__xdata uint16_t 			Key_FreeBuf[TOUCH_NUM];
__xdata UINT8V			Touch_IN;		

uint8_t TK_SelectChannel( uint8_t ch )
{
	if ( ch <= TOUCH_NUM )
	{
		TKEY_CTRL = ( TKEY_CTRL & 0XF8) | TK_Code[ch];
		return 1;
	}

	return	0;
}

uint8_t TK_Init( uint8_t channel)
{

__xdata	uint8_t 	i,j;
__xdata	uint16_t 	sum;
__xdata	uint16_t 	OverTime;
	
	P1_DIR_PU &= ~channel;
	P1_MOD_OC &= ~channel;
	TKEY_CTRL |= bTKC_2MS ;

	for ( i = 0; i < TOUCH_NUM; i++ )
	{
		sum = 0;
		j = SAMPLE_TIMES;
		TK_SelectChannel( i );
		while( j-- )
		{
			OverTime = 0;
			while( ( TKEY_CTRL & bTKC_IF ) == 0 )
			{
				if( ++OverTime == 0 )
				{
					return 0;
				}
			}
			sum += TKEY_DAT;												
		}
		Key_FreeBuf[i] = sum / SAMPLE_TIMES;
	}
	IE_TKEY = 1;    
	return 1;
}

void	TK_int_ISR( void ) __interrupt (INT_NO_TKEY)
{
__xdata	static uint8_t ch = 0;
__xdata	uint16_t KeyData;
	KeyData = TKEY_DAT;
	
	if( KeyData < ( Key_FreeBuf[ch] - TH_VALUE ) )
	{
		Touch_IN |=  1 << ( TK_Code[ch] - 1 );
	}
	if( ++ch >= TOUCH_NUM )
	{
		ch = 0;
	}	
	TK_SelectChannel( ch );
}

                                           

#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0

#define 	THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE
#define		BUFFER_SIZE				64
#define 	DUAL_BUFFER_SIZE		128
#define 	UsbSetupBuf     		((PUSB_SETUP_REQ)Ep0Buffer)
#define		L_WIN 					0X08
#define 	L_ALT 					0X04
#define		L_SHIFT					0X02
#define 	L_CTL					0X01
#define 	R_WIN 					0X80
#define 	R_ALT 					0X40
#define 	R_SHIFT					0X20
#define 	R_CTL					0X10
#define 	SPACE					0X2C
#define		ENTER					0X28

#define MOUSE 0hallowiegehts4vcxdrlpt8hallowiegehts4hallowiegehts5


__code uint8_t DevDesc[18] = {0x12,0x01,0x10,0x01,0x00,0x00,0x00,0x08,
                      0x3d,0x41,0x07,0x21,0x00,0x00,0x00,0x00,
                      0x00,0x01
                     };
__code uint8_t CfgDesc[59] =
{
    0x09,0x02,0x3b,0x00,0x02,0x01,0x00,0xA0,0x32,             //??????????
    0x09,0x04,0x00,0x00,0x01,0x03,0x01,0x01,0x00,             //??????????,????
    0x09,0x21,0x11,0x01,0x00,0x01,0x22,0x3e,0x00,             //HID????????
    0x07,0x05,0x81,0x03,0x08,0x00,0x0a,                       //??????????
    0x09,0x04,0x01,0x00,0x01,0x03,0x01,0x02,0x00,             //??????????,????
    0x09,0x21,0x10,0x01,0x00,0x01,0x22,0x34,0x00,             //HID????????
    0x07,0x05,0x82,0x03,0x04,0x00,0x0a                        //??????????
};
__code uint8_t KeyRepDesc[62] =
{
    0x05,0x01,0x09,0x06,0xA1,0x01,0x05,0x07,
    0x19,0xe0,0x29,0xe7,0x15,0x00,0x25,0x01,
    0x75,0x01,0x95,0x08,0x81,0x02,0x95,0x01,
    0x75,0x08,0x81,0x01,0x95,0x03,0x75,0x01,
    0x05,0x08,0x19,0x01,0x29,0x03,0x91,0x02,
    0x95,0x05,0x75,0x01,0x91,0x01,0x95,0x06,
    0x75,0x08,0x26,0xff,0x00,0x05,0x07,0x19,
    0x00,0x29,0x91,0x81,0x00,0xC0
};
__code uint8_t MouseRepDesc[52] =
{
    0x05,0x01,0x09,0x02,0xA1,0x01,0x09,0x01,
    0xA1,0x00,0x05,0x09,0x19,0x01,0x29,0x03,
    0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x03,
    0x81,0x02,0x75,0x05,0x95,0x01,0x81,0x01,
    0x05,0x01,0x09,0x30,0x09,0x31,0x09,0x38,
    0x15,0x81,0x25,0x7f,0x75,0x08,0x95,0x03,
    0x81,0x06,0xC0,0xC0
};

uint8_t HIDMouse[4] = {0x0,0x0,0x0,0x0};
uint8_t HIDKey[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

void CH554SoftReset( )
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    GLOBAL_CFG	|=bSW_RESET;
}

void CH554USBDevWakeup( )
{
  UDEV_CTRL |= bUD_LOW_SPEED;
  mDelaymS(2);
  UDEV_CTRL &= ~bUD_LOW_SPEED;	
}

void USBDeviceInit()
{
	  IE_USB = 0;
	  USB_CTRL = 0x00;                                                           // ??????USB????????
    UEP2_DMA = (uint16_t)Ep2Buffer;                                                      //????2????????????
    UEP2_3_MOD = UEP2_3_MOD & ~bUEP2_BUF_MOD | bUEP2_TX_EN;                    //????2???????? 64??????????
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                                 //????2????????????????????IN????????NAK
    UEP0_DMA = (uint16_t)Ep0Buffer;                                                      //????0????????????
    UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN);                                //????0??64??????????????
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                                 //OUT????????ACK??IN????????NAK
    UEP1_DMA = (uint16_t)Ep1Buffer;                                                      //????1????????????
    UEP4_1_MOD = UEP4_1_MOD & ~bUEP1_BUF_MOD | bUEP1_TX_EN;                    //????1???????? 64??????????
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                                 //????1????????????????????IN????????NAK	

	  USB_DEV_AD = 0x00;
	  UDEV_CTRL = bUD_PD_DIS;                                                    // ????DP/DM????????
	  USB_CTRL = bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;                      // ????USB??????DMA????????????????????????????????????NAK
	  UDEV_CTRL |= bUD_PORT_EN;                                                  // ????USB????
	  USB_INT_FG = 0xFF;                                                         // ??????????
	  USB_INT_EN = bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST;
	  IE_USB = 1;
}

void Enp1IntIn( )
{
    memcpy( Ep1Buffer, HIDKey, sizeof(HIDKey));                             
    UEP1_T_LEN = sizeof(HIDKey);                                             
    UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;               
}

void Enp2IntIn( )
{
    memcpy( Ep2Buffer, HIDMouse, sizeof(HIDMouse));                             
    UEP2_T_LEN = sizeof(HIDMouse);                                           
    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;              
}

void DeviceInterrupt(void) __interrupt (INT_NO_USB)	
{
    uint8_t len = 0;
    if(UIF_TRANSFER)                                                            //USB????????????
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 2:                                                  //endpoint 2# ????????????
            UEP2_T_LEN = 0;                                                     //????????????????????????
//            UEP1_CTRL ^= bUEP_T_TOG;                                          //????????????????????????????????
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //????????NAK
            break;
        case UIS_TOKEN_IN | 1:                                                  //endpoint 1# ????????????
            UEP1_T_LEN = 0;                                                     //????????????????????????
//            UEP2_CTRL ^= bUEP_T_TOG;                                          //????????????????????????????????
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //????????NAK
            FLAG = 1;                                                           /*????????????*/
            break;
        case UIS_TOKEN_SETUP | 0:                                                //SETUP????
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
                if(UsbSetupBuf->wLengthH || SetupLen > 0x7F )
                {
                    SetupLen = 0x7F;    // ??????????
                }
                len = 0;                                                        // ??????????????????0????
                SetupReq = UsbSetupBuf->bRequest;								
                if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )/* HID?????? */
                {
									switch( SetupReq ) 
									{
										case 0x01://GetReport
												 break;
										case 0x02://GetIdle
												 break;	
										case 0x03://GetProtocol
												 break;				
										case 0x09://SetReport										
												 break;
										case 0x0A://SetIdle
												 break;	
										case 0x0B://SetProtocol
												 break;
										default:
												 len = 0xFF;  								 					            /*??????????*/					
												 break;
								  }	
                }
                else
                {//????????
                    switch(SetupReq)                                        //??????
                    {
                    case USB_GET_DESCRIPTOR:
                        switch(UsbSetupBuf->wValueH)
                        {
                        case 1:                                             //??????????
                            pDescr = DevDesc;                               //??????????????????????????????
                            len = sizeof(DevDesc);
                            break;
                        case 2:                                             //??????????
                            pDescr = CfgDesc;                               //??????????????????????????????
                            len = sizeof(CfgDesc);
                            break;
                        case 0x22:                                          //??????????
                            if(UsbSetupBuf->wIndexL == 0)                   //????0??????????
                            {
                                pDescr = KeyRepDesc;                        //????????????
                                len = sizeof(KeyRepDesc);
                            }
                            else if(UsbSetupBuf->wIndexL == 1)              //????1??????????
                            {
                                pDescr = MouseRepDesc;                      //????????????
                                len = sizeof(MouseRepDesc);
                                Ready = 1;                                  //????????????????????????????????????????????????????????
                            }
                            else
                            {
                                len = 0xff;                                 //??????????2????????????????????????????
                            }
                            break;
                        default:
                            len = 0xff;                                     //????????????????????
                            break;
                        }
                        if ( SetupLen > len )
                        {
                            SetupLen = len;    //??????????
                        }
                        len = SetupLen >= 8 ? 8 : SetupLen;                  //????????????
                        memcpy(Ep0Buffer,pDescr,len);                        //????????????
                        SetupLen -= len;
                        pDescr += len;
                        break;
                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL;                     //????USB????????
                        break;
                    case USB_GET_CONFIGURATION:
                        Ep0Buffer[0] = UsbConfig;
                        if ( SetupLen >= 1 )
                        {
                            len = 1;
                        }
                        break;
                    case USB_SET_CONFIGURATION:
                        UsbConfig = UsbSetupBuf->wValueL;
                        break;
                    case 0x0A:
                        break;
                    case USB_CLEAR_FEATURE:                                            //Clear Feature
                        if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// ????
                        {
                            switch( UsbSetupBuf->wIndexL )
                            {
                            case 0x82:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x81:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x01:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                                break;
                            default:
                                len = 0xFF;                                            // ????????????
                                break;
                            }
                        }
                        if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )// ????
                        {
													break;
                        }													
                        else
                        {
                            len = 0xFF;                                                // ??????????????
                        }
                        break;
                    case USB_SET_FEATURE:                                              /* Set Feature */
                        if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x00 )             /* ???????? */
                        {
                            if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                            {
                                if( CfgDesc[ 7 ] & 0x20 )
                                {
                                    /* ???????????????? */
                                }
                                else
                                {
                                    len = 0xFF;                                        /* ???????? */
                                }
                            }
                            else
                            {
                                len = 0xFF;                                            /* ???????? */
                            }
                        }
                        else if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x02 )        /* ???????? */
                        {
                            if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
                            {
                                switch( ( ( uint16_t )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
                                {
                                case 0x82:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ????????2 IN STALL */
                                    break;
                                case 0x02:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* ????????2 OUT Stall */
                                    break;
                                case 0x81:
                                    UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ????????1 IN STALL */
                                    break;
                                default:
                                    len = 0xFF;                               //????????
                                    break;
                                }
                            }
                            else
                            {
                                len = 0xFF;                                   //????????
                            }
                        }
                        else
                        {
                            len = 0xFF;                                      //????????
                        }
                        break;
                    case USB_GET_STATUS:
                        Ep0Buffer[0] = 0x00;
                        Ep0Buffer[1] = 0x00;
                        if ( SetupLen >= 2 )
                        {
                            len = 2;
                        }
                        else
                        {
                            len = SetupLen;
                        }
                        break;
                    default:
                        len = 0xff;                                           //????????
                        break;
                    }
                }
            }
            else
            {
                len = 0xff;                                                   //??????????
            }
            if(len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
            }
            else if(len)                                                //????????????????????????0??????
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//????????????DATA1??????????ACK
            }
            else
            {
                UEP0_T_LEN = 0;  //????????????????????????????????????0??????????????????????????????????
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//????????????DATA1,????????ACK
            }
            break;
        case UIS_TOKEN_IN | 0:                                               //endpoint0 IN
            switch(SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= 8 ? 8 : SetupLen;                          //????????????
                memcpy( Ep0Buffer, pDescr, len );                            //????????????
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;                                     //??????????????
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0;                                              //??????????????????????????????0??????????????????????
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
            len = USB_RX_LEN;
            if(SetupReq == 0x09)
            {
                if(Ep0Buffer[0])
                {
                    numlock = 1;   
                }
                else if(Ep0Buffer[0] == 0)
                {
                    
                    numlock = 0;   
                }				
            }
            UEP0_CTRL ^= bUEP_R_TOG;                                     //??????????????						
            break;
        default:
            break;
        }
        UIF_TRANSFER = 0;                                                 //??0????????
    }
    if(UIF_BUS_RST)                                                       //????????USB????????????
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK;
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0;                                                 //?
    }
    if (UIF_SUSPEND)                                                     //USB????????/????????
    {
        UIF_SUSPEND = 0;
        if ( USB_MIS_ST & bUMS_SUSPEND )                                 //?
        {
        }
    }
    else {                                                               //??????????,????????????????
        USB_INT_FG = 0xFF;        
    }
}


static void SendKey ( char *p )
{

	char c = *p;
	char d = 0;
		
	if( (c >= 'a') && (c <= 'z' )){
		c = c - 'a' + 'A';
		d=1;
	}
	if(d == 0)HIDKey[0] = L_SHIFT;
	if( (c >= 'A') && (c <= 'Z' )){
		HIDKey[2] = c - 'A' + 4;
	}
	else
		if( c >= '1' && c <= '9' ){
				HIDKey[0] = 0x000;
		HIDKey[2] = c - '1' + 0X1E;}
		else
		{
		switch ( c ){
			case '`' :
				HIDKey[0] = 0X08;
				HIDKey[2] = 0X15;
				break;
			case '\\':
				HIDKey[2] = 0x31;
				break;
			case ' ':
				HIDKey[2] = SPACE;
				break;
			case '\r':
				HIDKey[2] = ENTER;
				break;
			case ':':
				HIDKey[0] = 0x02;
				HIDKey[2] = 0x33;
				break;
			case '+':
				HIDKey[0] = 0x000;
				HIDKey[2] = 0x57;
				break;
			case '?':
				HIDKey[0] = L_SHIFT;
				HIDKey[2] = 0x38;
				break;
			case '_':
				HIDKey[0] = 0X02;
				HIDKey[2] = 0X2D;
				break;
			case '/':
				HIDKey[0] = L_CTL + L_ALT;
				HIDKey[2] = 0X16;
				break;
			case '0':
				HIDKey[2] = 0X27;
				break;
			case '.':
				HIDKey[2] = 0X37;
				break;
			case '~':
				HIDKey[0] = L_ALT;
				HIDKey[2] = 0X05;
				break;
			case '!':
				HIDKey[0] = L_ALT;
				HIDKey[2] = 0X08;
				break;
			default:
				break;
		}
	}
	mDelaymS( 10 );																			
	while(FLAG == 0);                   								
	Enp1IntIn();						
	while(FLAG == 0);   																					
	mDelaymS( 10 );
	HIDKey[0] = 0X00;     						
	HIDKey[2] = 0X00;                                              								
	while(FLAG == 0);                                           						
	Enp1IntIn();			
	while(FLAG == 0); 
}


void HIDValueHandle()
{
if( readFlag == 1 )		
		{ 	 	
       		SendKey(pStr);																		
			pStr++;	
			if(*pStr == '\0')			
			{
				readFlag = 0;
				b=0;	
			}		
		}
		else			
		{
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;   
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;  
			if(b == 1){     
				b=0;			
				pStr = sPath;
				readFlag=1;			
				}			
		}	
	
}


main()
{
    CfgFsys( );                                         
    mDelaymS(5);                                                         
    mInitSTDIO( );      	
    USBDeviceInit();   
	TK_Init( BIT4+BIT5);	
	TK_SelectChannel(0);	
	
    P3_MOD_OC = P3_MOD_OC |(1<<LED_PIN1);
    P3_DIR_PU = P3_DIR_PU |	(1<<LED_PIN1);
    P3_MOD_OC = P3_MOD_OC |(1<<LED_PIN2);
    P3_DIR_PU = P3_DIR_PU |	(1<<LED_PIN2);

	
	TMOD = 0x11;
	TH0 = (65536 - 2000)/256;  // f??r Startwert 
	TL0 = (65536 - 2000)%256;  // f??r Startwert 
	TR0 = 1;    // Timer 0 starten  - TR1 f??r Timer 1
	ET0 = 1;    // Interrupt f??r Timer 0 aktivieren
	EA  = 1;    // Globalen Interrupt aktivieren -- ab jetzt geht's rund                                                   
    UEP1_T_LEN = 0;                                                     
    UEP2_T_LEN = 0;                                                      
    FLAG = 0;
    Ready = 0;
	b=0; 
    while(1)
    {
	if (millis-last>40){
		 LED1 = !LED1;
		 last=millis;         
	LED2 = numlock;
		 if( Touch_IN != 0 )
			{
			if( Touch_IN & CH2 )jump_to_bootloader();
			if( Touch_IN & CH3 )b=1;
			Touch_IN = 0;
			}
	
	
        if(Ready)
        {
            HIDValueHandle();
        }           
	}                                   
    }              
       
}