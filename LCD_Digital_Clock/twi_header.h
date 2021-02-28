#ifndef twi_header
#define twi_header

// Two wire interface or I2C interface works on two pins SCL and SDA
// SCL is the serial clock and SDA is the serial data
// Important to read device data sheet to understand how a particular device communicates with uC using TWI/I2C

#include <util/delay.h>
#include <avr/io.h>
#include <util/twi.h>

/***************************************TWI/I2C Functions*************************************/
void twi_init(void);
void twi_start(void);
void twi_stop(void);
void twi_rstart(void);
void twi_SLA_W(uint8_t Slave_Write_Addr);
void twi_SLA_R(uint8_t Slave_Read_Addr);
void twi_SendData(uint8_t Data_Byte);
uint8_t twi_ReadNAck(void);
uint8_t twi_ReadRegister(uint8_t Register_Adrress);
void twi_Write_Register(uint8_t Register_Address, uint8_t Register_Data);
void Encode_time(void);

uint8_t ACK_Err = 0;

void twi_init(void)
{
	TWBR = 2;               // Setting SCL to 50 KHz 
	TWCR |= 1 << TWEN;      // Enables TWI operation and activates TWI 
}

void twi_start(void)
{
	
	TWCR = (1 << TWINT | 1 << TWSTA | 1 << TWEN); // Transmit Start condition
	while(!(TWCR & (1 << TWINT)));				  // Wait for TWINT Flag set
	while(((TWSR & 0xF8) != 0x08) && ((TWSR & 0xF8) != 0x10)); // Check value of TWI Status Register. Mask prescaler bits. 
															   // 08 - Start Condition Transmitted, 10 - Repeated START condition transmitted 
}

void twi_stop(void)
{
	TWCR = (1 << TWEN | 1 << TWSTO | 1 << TWINT); // Transmit Stop Condition
}

void twi_rstart(void)
{
	TWCR = (1 << TWINT | 1 << TWSTA | 1 << TWEN); // Transmit Start condition
	while(!(TWCR & (1 << TWINT)));				  // Wait for TWINT Flag set
	while((TWSR & 0xF8) != 0x10);				  // Check value of TWI Status Register. Mask prescaler bits. 10 - Repeated START condition transmitted 
}

void twi_SLA_W(uint8_t Slave_Write_Addr)
{   
	
	TWDR = Slave_Write_Addr;					// Indicate which slave device is to be accessed for write
	TWCR = (1 << TWEN | 1 << TWINT);			// Clear TWINT bit in TWCR to start transmission of address
	while(!(TWCR & (1 << TWINT)));				// Wait for TWINT Flag set
	while(((TWSR & 0xF8) != 0x18) && ((TWSR & 0xF8) != 0x20)); // Check value of TWI Status Register. Mask prescaler bits.
															   // 18 - SLA+W has been transmitted; ACK has been received
															   // 20 - SLA+W has been transmitted; NOT ACK has been received
	if ((TWSR & 0xF8) == 0x20) ACK_Err = 1;
}

void twi_SLA_R(uint8_t Slave_Read_Addr)
{
	TWDR = Slave_Read_Addr;						// Indicate which slave device is to be addressed for read
	TWCR =(1 << TWEN | 1 << TWINT);				// Clear TWINT bit in TWCR to start transmission of address
	while(!(TWCR & (1 << TWINT)));				// Wait for TWINT Flag set
	while(((TWSR & 0xF8) != 0x40) && ((TWSR & 0xF8) != 0x48));	// Check value of TWI Status Register. Mask prescaler bits.
																// 40 - SLA+R has been transmitted; ACK has been received
																// 48 - SLA+R has been transmitted; NOT ACK has been received
	
	if((TWSR & 0xF8) == 0x48) ACK_Err = 1;
}

void twi_SendData(uint8_t Data_Byte)
{
	
	TWDR = Data_Byte;						// Load DATA into TWDR Register
	TWCR = (1 << TWEN | 1 << TWINT);		// Clear TWINT bit in TWCR to start transmission of data
	while(!(TWCR & (1 << TWINT)));			// Wait for TWINT Flag set
	while(((TWSR & 0xF8) != 0x28) && ((TWSR & 0xF8) != 0x30)); // 28 - Data byte transmitted; ACK received
															   // 30 - Data byte transmitted; NACK received
	
	if((TWSR & 0xF8) == 0x30) ACK_Err = 1;

}

uint8_t twi_ReadNAck(void)
{
	TWCR = (1 << TWINT | 1 << TWEN); // Clear TWINT bit in TWCR to start reception of data
	while(!(TWCR & (1<<TWINT)));	 // Wait for TWINT Flag set
	return TWDR;					 // Return Data register
}
uint8_t twi_ReadRegister(uint8_t Register_Adrress)
{
	twi_start();					// Send Start Condition
	twi_SLA_W(0xD0);				// SLA+W transmit. 7-Bit slave address for DS1307 is 1101000 followed by R/W bit i.e. 0 for Write. 0b11010000 or 0xD0
	twi_SendData(Register_Adrress); // Write the address of register from which uC wants to read
	twi_rstart();					// Send Repeated start
	twi_SLA_R(0xD1);				// SLA+R. 7-Bit slave address for DS1307 is 1101000 followed by R/W bit i.e. 1 for Read. 0b11010001 or 0xD1
	
    uint8_t temp_value = twi_ReadNAck(); // Read data register into local variable
	twi_stop();							 // Send Stop Condition 
	
	if (ACK_Err)
	{
		return 0XFF;				// Return 0xFF if there is any error in any step
	}
	else return temp_value;			// Return Read Data
}
void twi_Write_Register(uint8_t Register_Address, uint8_t Register_Data)
{
	twi_start();					// Send Start Condition
	twi_SLA_W(0xD0);				// SLA+W transmit. 7-Bit slave address for DS1307 is 1101000 followed by R/W bit i.e. 0 for Write. 0b11010000 or 0xD0
	twi_SendData(Register_Address); // Write the address of register from which uC wants to write into
	twi_SendData(Register_Data);	// Send Data to be written
	twi_stop();						// Send Stop Condition
}

#endif