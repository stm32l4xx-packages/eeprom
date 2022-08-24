/*
 * eeprom.cpp
 *
 *  Created on: Dec 11, 2020
 *      Author: jsilva
 */

#include "eeprom.h"

extern I2C_HandleTypeDef hi2c1;
uint8_t id = AT24_DEV_ADDR;

uint8_t read_byte_i2c(uint8_t partition, uint16_t address){

	HAL_Delay(10);
	uint8_t res = 0;

	uint8_t slave_id = id|(partition&0x0F);
	/*##-5- Master sends read request for slave ##############################*/
	//hi2c1.Init.OwnAddress1 = AT24_DEV_ADDR;	// slave id

	/* Update bTransferRequest to send buffer read request for Slave */
	uint8_t bTransferRequest[2];
	bTransferRequest[0] = (uint8_t)(address>>8);
	bTransferRequest[1] = (uint8_t)address;

	#ifdef DEBUG_I2C
		sprintf(logging.log_msg,"i2c read >> address 0x%x \n",address);
		logging.log_nnl(logging.log_msg,"sensors");
	#endif

	do{
		res = HAL_I2C_Master_Transmit(&hi2c1, slave_id, (uint8_t*)&bTransferRequest, 2,300);
		if( res != HAL_OK){
			#ifdef DEBUG_I2C
				sprintf(logging.log_msg,"!! error i2c >> transmitting data... \n");
				logging.log(logging.log_msg,"sensors");
			#endif
			//Error_Handler(__FILE__, __LINE__);
			return false;
		}
		while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY){}
	}while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);

	uint8_t data[1];

	/*##-7- Master receives aRxBuffer from slave #############################*/
	do{

		if(HAL_I2C_Master_Receive(&hi2c1,slave_id|AT24_DEV_ADDR_READ,data,1,1000)!=HAL_OK){
		//if(HAL_I2C_Master_Receive(&hi2c1, (uint8_t)id, (uint8_t*)data, s->len,2000)!= HAL_OK){
			/* Error_Handler() function is called when error occurs. */
			//Error_Handler(__FILE__, __LINE__);
			sprintf(logging.log_msg,"!! error i2c >> res: 0x%x - restarting I2C \n",res);
			logging.log(logging.log_msg,"sensors");
			HAL_I2C_DeInit(&hi2c1);
			HAL_I2C_Init(&hi2c1);
			return false;
		}

		while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
		{
		}

	}while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);


	#ifdef DEBUG_I2C
		uint8_t i = 0;
		sprintf(logging.log_msg,"i2c read >> data received: 0x%x", data[0]);
		logging.log(logging.log_msg,"sensors");
	#endif

	return true;
}

bool write_byte_i2c(uint8_t partition, uint16_t address, uint8_t byte){

	/*##-5- Master sends read request for slave ##############################*/
	//hi2c1.Init.OwnAddress1 = AT24_DEV_ADDR;	// slave id
	uint8_t slave_id = id|(partition&0x0F);

	/* Update bTransferRequest to send buffer read request for Slave */
	uint8_t res = 0;

	uint8_t data[3];
	data[0] = (uint8_t)(address>>8);
	data[1] = address;
	data[2] = byte;
	#ifdef DEBUG_I2C
		sprintf(logging.log_msg,"i2c write >> on address: 0x%x, data: 0x%2x",address,data[2]);
		logging.log(logging.log_msg,"eeprom");
	#endif

	do
	{
		res = HAL_I2C_Master_Transmit(&hi2c1, slave_id, (uint8_t*)data, 3,100);
		if( res != HAL_OK)
		{
			#ifdef DEBUG_I2C
				sprintf(logging.log_msg,"!! error i2c >> transmitting res: 0x%x \n",res);
				logging.log(logging.log_msg,"sensors");
			#endif
			/* Error_Handler() function is called when error occurs. */
			//Error_Handler(__FILE__, __LINE__);
			return false;
		}

		/*  Before starting a new communication transfer, you need to check the current
				state of the peripheral; if it�s busy you need to wait for the end of current
				transfer before starting a new one.
				For simplicity reasons, this example is just waiting till the end of the
				transfer, but application may perform other tasks while transfer operation
				is ongoing. */

		while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
		{
		}


		/* When Acknowledge failure occurs (Slave don't acknowledge it's address)
			 Master restarts communication */
	}while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);


	return true;

}

bool write_sequential_i2c(uint8_t partition, uint16_t address, uint8_t* data, uint8_t len){

	/*##-5- Master sends read request for slave ##############################*/
	uint8_t slave_id = id|(partition&0x0F);

	/* Update bTransferRequest to send buffer read request for Slave */
	uint8_t res = 0;

	uint8_t* transfer = (uint8_t*)malloc(len+2);
	if(transfer == nullptr) return false;

	memcpy(&transfer[2],data,len);

	transfer[0] = (uint8_t)(address>>8);
	transfer[1] = address;
	len += 2;

	#ifdef DEBUG_I2C
		uint8_t i = 0;
		sprintf(logging.log_msg,"i2c write >> on address: 0x%x, data:",address);
		logging.log(logging.log_msg,"eeprom");

		while(i<len){
			if(i==len-1)
				sprintf(logging.log_msg,"0x%x\n",transfer[i++]);
			else
				sprintf(logging.log_msg,"0x%x ",transfer[i++]);
			logging.log_nnl(logging.log_msg,"eeprom");
		}
	#endif

	uint8_t tries = 0;
	bool error = true;
	while(tries < 3){

		do{

			/*  Before starting a new communication transfer, you need to check the current
					state of the peripheral; if it�s busy you need to wait for the end of current
					transfer before starting a new one.
					For simplicity reasons, this example is just waiting till the end of the
					transfer, but application may perform other tasks while transfer operation
					is ongoing. */

			while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY){}

			res = HAL_I2C_Master_Transmit(&hi2c1, slave_id, (uint8_t*)transfer, len,1000);
			if( res != HAL_OK){
				#ifdef DEBUG_I2C
					sprintf(logging.log_msg,"!! error i2c >> transmitting res: 0x%x \n",res);
					logging.log_nnl(logging.log_msg,"sensors");
				#endif
				/* Error_Handler() function is called when error occurs. */
				//Error_Handler(__FILE__, __LINE__);
				//return false;
			}else error = false;
			/* When Acknowledge failure occurs (Slave don't acknowledge it's address)
				 Master restarts communication */
		}while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);

		HAL_Delay(10);

		if(error) tries++;
		else break;
	}

	free(transfer);

	if(tries < 3) return true;

	return false;

}

bool read_sequential_i2c(uint8_t partition, uint16_t address, uint8_t* data, uint8_t len){

	HAL_Delay(10);
	uint8_t res = 0;

	/*##-5- Master sends read request for slave ##############################*/
	//hi2c1.Init.OwnAddress1 = AT24_DEV_ADDR;	// slave id
	uint8_t slave_id = id|(partition&0x0F);

	/* Update bTransferRequest to send buffer read request for Slave */
	uint8_t bTransferRequest[2];
	bTransferRequest[0] = (uint8_t)(address>>8);
	bTransferRequest[1] = (uint8_t)address;

	#ifdef DEBUG_I2C
		sprintf(logging.log_msg,"i2c read >> address 0x%x \n",address);
		logging.log_nnl(logging.log_msg,"sensors");
	#endif

	do{
		res = HAL_I2C_Master_Transmit(&hi2c1, slave_id, (uint8_t*)&bTransferRequest, 2,300);
		if( res != HAL_OK){
			#ifdef DEBUG_I2C
				sprintf(logging.log_msg,"!! error i2c >> transmitting data... \n");
				logging.log(logging.log_msg,"sensors");
			#endif
			//Error_Handler(__FILE__, __LINE__);
			return false;
		}
		while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY){}
	}while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);

	/*##-7- Master receives aRxBuffer from slave #############################*/
	do{

		if(HAL_I2C_Master_Receive(&hi2c1,slave_id|AT24_DEV_ADDR_READ,data,len,1000)!=HAL_OK){
		//if(HAL_I2C_Master_Receive(&hi2c1, (uint8_t)id, (uint8_t*)data, s->len,2000)!= HAL_OK){
			/* Error_Handler() function is called when error occurs. */
			//Error_Handler(__FILE__, __LINE__);
			#ifdef DEBUG_I2C
			sprintf(logging.log_msg,"!! error i2c >> res: 0x%x - restarting I2C \n",res);
			logging.log(logging.log_msg,"sensors");
			#endif
			HAL_I2C_DeInit(&hi2c1);
			HAL_I2C_Init(&hi2c1);
			return false;
		}

		while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
		{
		}

	}while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);


	#ifdef DEBUG_I2C
		uint8_t i = 0;
		sprintf(logging.log_msg,"i2c read >> on address: 0x%x, data:",address);
		logging.log(logging.log_msg,"eeprom");

		while(i<len){
			if(i==len-1)
				sprintf(logging.log_msg,"0x%x\n",data[i++]);
			else
				sprintf(logging.log_msg,"0x%x ",data[i++]);
			logging.log_nnl(logging.log_msg,"eeprom");
		}
	#endif

	return true;
}
