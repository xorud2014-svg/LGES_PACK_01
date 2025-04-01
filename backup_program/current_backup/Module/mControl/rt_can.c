//rt_can_fd
#include <linux/ioport.h>
#include <linux/module.h>

#include <asm/io.h>
#include <pthread.h>

#include "rt_can.h"
#include "rt_can_ext.h"

#include "../../INC/datastore.h"
extern S_SYSTEM_DATA	*myData;

unsigned char log_ch = 0;

#ifdef __SBC_EM104_A5362__
struct rt_can_fd_struct rt_can_fd_table[RT_CAN_FD_CNT] =
{ //sbc : a5362
    {0, RT_CAN_FD_BASE_BAUD, 0x810, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {1, RT_CAN_FD_BASE_BAUD, 0x818, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {2, RT_CAN_FD_BASE_BAUD, 0x820, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {3, RT_CAN_FD_BASE_BAUD, 0x828, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {4, RT_CAN_FD_BASE_BAUD, 0x830, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {5, RT_CAN_FD_BASE_BAUD, 0x838, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {6, RT_CAN_FD_BASE_BAUD, 0x840, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {7, RT_CAN_FD_BASE_BAUD, 0x848, 5, RT_CAN_FD_CLASSIC_FLAG, 0, 1}
};
#else
struct rt_can_fd_struct rt_can_fd_table[RT_CAN_FD_CNT] =
{ //sbc : mark533, mark800, v621
    {0, RT_CAN_FD_BASE_BAUD, 0x810, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {1, RT_CAN_FD_BASE_BAUD, 0x818, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {2, RT_CAN_FD_BASE_BAUD, 0x820, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {3, RT_CAN_FD_BASE_BAUD, 0x828, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {4, RT_CAN_FD_BASE_BAUD, 0x830, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {5, RT_CAN_FD_BASE_BAUD, 0x838, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {6, RT_CAN_FD_BASE_BAUD, 0x840, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1},
    {7, RT_CAN_FD_BASE_BAUD, 0x848, 9, RT_CAN_FD_CLASSIC_FLAG, 0, 1}
};
#endif

int rt_can_read(int num, char *ptr)
{
	int idx;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error1 %d, %d\n",
			num, p->used);
		return -2;
	}
	if(p->rx_msg_count == 0) {
		//printk(KERN_WARNING "rt_can_fd: rt_can_read error2a %d, %d\n",
		//	num, p->rx_msg_count);
		return -3;
	} else if(p->rx_msg_count < 0) {
		p->rx_msg_count = 0;
		printk(KERN_WARNING "rt_can_fd: rt_can_read error2b %d, %d\n",
			num, p->rx_msg_count);
		return -3;
	}

	if(p->rx_msg_write_idx == p->rx_msg_read_idx) {
		//printk(KERN_WARNING "rt_can_fd: rt_can_read error3 %d, %d, %d\n",
		//	num, p->rx_msg_write_idx, p->rx_msg_read_idx);
		p->rx_msg_count = 0;
		return -4;
	}

	p->rx_msg_read_idx++;
	if(p->rx_msg_read_idx >= RT_CAN_FD_RX_MSG_BUF_SIZE) p->rx_msg_read_idx = 0;

	idx = p->rx_msg_read_idx;
	memcpy(ptr, (char *)&p->rx_buf[idx], sizeof(struct rt_can_fd_msg));
	p->rx_msg_count--;

    return p->rx_msg_count;
}

int rt_can_write(int num, const char *msg)
{
	int idx;
	struct rt_can_fd_struct *p;

	if(max_can_fd_ch == 0) return 0;

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error1 %d, %d\n",
			num, p->used);
		return -2;
	}

	if(p->set_phase != PH100) return -3;

	if(p->tx_msg_count >= RT_CAN_FD_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			//printk(KERN_WARNING
			//	"rt_can_fd: rt_can_write error2 %d, %d, %d\n", num,
			//	p->tx_msg_count, p->tx_msg_error);
			return -3;
		} else {
			//printk(KERN_WARNING
			//	"rt_can_fd: rt_can_write error3 %d, %d, %d\n", num,
			//	p->tx_msg_count, p->tx_msg_error);

			p->tx_msg_error = 0;
			p->tx_msg_count = 0;
			p->tx_msg_read_idx = p->tx_msg_write_idx;
		}
	}

	idx = p->tx_msg_write_idx;
	idx++;
	if(idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) idx = 0;

	memcpy((char *)&p->tx_buf[idx], msg, sizeof(struct rt_can_fd_msg));

	if(p->flag == RT_CAN_FD_CLASSIC_FLAG) {
		p->tx_buf[idx].can_fd_flag = 0;
	} else { //RT_CAN_FD_NORMAL_FLAG
		p->tx_buf[idx].can_fd_flag = 1;
	}

	p->tx_msg_write_idx = idx;
	p->tx_msg_count++;

	return 0;
}

int rx_can_data_11(int num)
{
	char error;
	int rtn = 0;
    struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(p->set_phase != PH100) {
		return 10;
	}

	if(p->tmp_set_phase == PH1 && p->chip_rx_phase == PH0
		&& p->chip_tx_phase == PH0) {
		p->set_phase = PH0;
		return 50;
	}

	if(p->tmp_set_phase == PH1 && p->chip_rx_phase == PH0) {
		return 10;
	}

	if(p->chip_rx_phase == PH0 && myData->mData.signal[M_SIG_WDT_PHASE] == P2) {
		return 10;
	}

	if(p->chip_tx_phase != PH0) {
		return 10;
	}

	switch(p->chip_rx_phase) {
		case PH0:
			p->chip_tx_rx_phase = PH3;
			p->chip_transfer_phase = PH0;
			p->chip_rx_phase++;
			//skip break;
		case PH1:
			error = CHIP_MCU_get_status(num, 0);
			if(error < 0) { //fail
				p->chip_tx_rx_phase = PH4;
				p->chip_rx_phase = PH0;
				rtn = 10; //kjg_240624
				break;
			} else if(error == 0) {
				break; //wait
			} else {} //end

			myData->test_val_i[num][0] = p->mcu_rx_msg_count;
			myData->test_val_i[num][1] = p->mcu_rx_msg_wr_idx;
			myData->test_val_i[num][2] = p->mcu_rx_msg_rd_idx;
			myData->test_val_i[num][3] = p->mcu_chip_op_phase;
			myData->test_val_i[num][4] = p->mcu_filter_id_count;

			if(p->mcu_rx_msg_count == 0) {
				p->chip_tx_rx_phase = PH4;
				p->chip_rx_phase = PH0;
				rtn = 10;
				break;
			}

			p->chip_transfer_phase = PH0;
			p->chip_rx_phase++;
			//skip break;
		case PH2:
			error = CHIP_MCU_ReceiveMessageGet(num);
			if(error < 0) { //fail
				p->chip_tx_rx_phase = PH4;
				p->chip_rx_phase = PH0;
				rtn = 10; //kjg_240624
				break;
			} else if(error == 0) {
				break; //wait
			} else {} //end

			if(error == 100) {
				myData->test_val_i[num][5]++;
				myData->test_val_i[num][12]
					= myData->test_val_i[num][11] - myData->test_val_i[num][5];

				rtn = 49;
			} else {
				rtn = 10; //kjg_240624
			}

			p->chip_tx_rx_phase = PH4;
			p->chip_rx_phase = PH0;
			break;
		default:
			break;
	}

	return rtn;
}

int tx_can_data_11(int num)
{
	char error;
	int idx, rtn = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(p->set_phase != PH100) return 10;

	if(p->tmp_set_phase == PH1 && p->chip_tx_phase == PH0
		&& p->chip_rx_phase == PH0) {
		p->set_phase = PH0;
		return 50;
	}

	if(p->tmp_set_phase == PH1 && p->chip_tx_phase == PH0) {
		return 10;
	}

	if(p->chip_tx_phase == PH0 && myData->mData.signal[M_SIG_WDT_PHASE] == P2) {
		return 10;
	}

	if(p->chip_rx_phase != PH0) {
		return 10;
	}

	switch(p->chip_tx_phase) {
		case PH0:
			p->chip_op_phase = PH0;

			if(p->tx_msg_count <= 0) {
				p->tx_msg_read_idx = p->tx_msg_write_idx;
				return 10;
			}

			p->chip_tx_rx_phase = PH1;

			idx = p->tx_msg_read_idx + 1;
			if(idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) idx = 0;

			//Initialize ID and Control bits
			p->txObj.word[0] = 0;
			p->txObj.word[1] = 0;

			if(p->type == 0) {
				p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
				p->txObj.bF.id.EID = 0; //Extended ID
				p->txObj.bF.ctrl.IDE = 0; //Standard frame
			} else {
				/*kjgw_240624 p->txObj.bF.id.SID = (p->tx_buf[idx].id >> 18) & 0x7FF;
				p->txObj.bF.id.EID = p->tx_buf[idx].id & 0x3FFFF;
				p->txObj.bF.ctrl.IDE = 1; //Extended frame*/
				if(p->tx_buf[idx].id < 0x800) {	//csk_240313
					p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
					p->txObj.bF.id.EID = 0; //Extended ID
					p->txObj.bF.ctrl.IDE = 0; //Standard frame
				} else {
					p->txObj.bF.id.SID = (p->tx_buf[idx].id >> 18) & 0x7FF;
					p->txObj.bF.id.EID = p->tx_buf[idx].id & 0x3FFFF;
					p->txObj.bF.ctrl.IDE = 1; //Extended frame
				}
			}

			if(p->tx_buf[idx].can_fd_flag == 0) {
				p->txObj.bF.ctrl.FDF = 0;
				p->txObj.bF.ctrl.BRS = 0;
			} else {
				p->txObj.bF.ctrl.FDF = 1; //CAN FD frame
				p->txObj.bF.ctrl.BRS = 1; //Switch bit rate
			}
			p->txObj.bF.ctrl.DLC = CHIP_DataBytesToDlc(p->tx_buf[idx].length);
			if(p->txObj.bF.ctrl.DLC == 0) {
				p->txObj.bF.ctrl.RTR = 1; //A remote frame request
			} else {
				p->txObj.bF.ctrl.RTR = 0; //Not a remote frame request
			}

			p->chip_transfer_phase = PH0;
			p->chip_tx_phase++;
			//skip break;
		case PH1:
			//Load message and transmit
			error = CHIP_MCU_TransmitMessage(num);
			if(error < 0) { //fail
				p->chip_tx_phase = PH0;
				p->chip_tx_rx_phase = PH2;
				rtn = 10; //kjg_240624
			} else if(error == 0) {
				break; //wait
			} else { //message sent
				myData->test_val_i[num][6] = p->mcu_rx_msg_count;
				myData->test_val_i[num][7] = p->mcu_rx_msg_wr_idx;
				myData->test_val_i[num][8] = p->mcu_rx_msg_rd_idx;
				myData->test_val_i[num][9] = p->mcu_chip_op_phase;
				myData->test_val_i[num][10] = p->mcu_filter_id_count;
				myData->test_val_i[num][11]++;
				myData->test_val_i[num][12]
					= myData->test_val_i[num][11] - myData->test_val_i[num][5];

				p->chip_tx_phase = PH0;
				p->chip_tx_rx_phase = PH2;
				rtn = 10; //kjg_240624
			}
			break;
		default:
			break;
	}

	return rtn;
}

int rt_can_set_param_0(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc, int terminal_r)
{
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(max_can_fd_ch == 0) return 0;

	if(max_can_fd_ch == 0 || num >= max_can_fd_ch) {
		printk(KERN_WARNING "rt_can_fd: rt_can_set_param_0 error %d %d\n",
			num, max_can_fd_ch);
		return -1;
	}

	p->tmp_set_phase = PH1;

	p->set_param_baud = baud;
	p->set_param_sjw = (unsigned char)sjw;
	p->set_param_ext = (unsigned char)ext;
	p->set_param_fd_flag = (unsigned char)fd_flag;
	p->set_param_data_rate = (unsigned char)data_rate;
	p->set_param_crc = (unsigned char)crc;
	p->set_param_terminal_r = (unsigned char)terminal_r;
	
	return 0;
}

int rt_can_set_param_11(int num)
{
	unsigned char tmp1, terminal_r;
	int error, rtn = 0, j;
	unsigned int base, o_addr1, o_addr2;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->set_phase) {
		case PH0:
			if(p->tmp_set_phase != PH1) break;
			p->tmp_set_phase = PH0;

			tmp1 = 0x10;
			if(p->set_param_baud == 500000 && p->set_param_data_rate == 5) {
				//500K_3M -> 500K_3M2
				if(num <= 3) {
					terminal_resistor[0] |= (tmp1 << num);
				} else {
					terminal_resistor[1] |= (tmp1 << (num - 4));
				}
			} else { //500K_3M
				if(num <= 3) {
					terminal_resistor[0] &= ~(tmp1 << num);
				} else {
					terminal_resistor[1] &= ~(tmp1 << (num - 4));
				}
			}

			terminal_r = p->set_param_terminal_r;

			tmp1 = 0x01;
			if(terminal_r == 0) {
				if(num <= 3) {
					terminal_resistor[0] &= ~(tmp1 << num);
				} else {
					terminal_resistor[1] &= ~(tmp1 << (num - 4));
				}
			} else {
				if(num <= 3) {
					terminal_resistor[0] |= (tmp1 << num);
				} else {
					terminal_resistor[1] |= (tmp1 << (num - 4));
				}
			}
			base = RT_CAN_FD_BASE_ADDR;
			o_addr1 = base + RT_CAN_FD_BD1_OUT_1;
			o_addr2 = base + RT_CAN_FD_BD2_OUT_1;
			outb(terminal_resistor[0], o_addr1);
			outb(terminal_resistor[1], o_addr2);

			tmp1 = 0x01;
			if(num <= 3) {
				tmp1 = tmp1 << num;
				o_addr1 = base + RT_CAN_FD_BD1_OUT_3;
			} else {
				tmp1 = tmp1 << (num - 4);
				o_addr1 = base + RT_CAN_FD_BD2_OUT_3;
			}
			outb(tmp1, o_addr1); //CAN_RESET set
			//usleep(10); //kjgw_240221
			for(j=0; j < 200; j++) {}
			outb(0x00, o_addr1); //CAN_RESET clear
			//usleep(10);
			for(j=0; j < 200; j++) {}

			p->rx_msg_write_idx = 0;
			p->rx_msg_read_idx = 0;
			p->rx_msg_count = 0;
			p->tx_msg_write_idx = 0;
			p->tx_msg_read_idx = 0;
			p->tx_msg_count = 0;
			p->tx_msg_error = 0;
			p->error = 0; //init. last error code
			p->error_count = 0;
			p->type = (int)p->set_param_ext;

			myData->CAN.chip_diff_1tick_count[num]
				= myData->mData.misc.timer_1tick_count;

			p->chip_transfer_phase = PH0;
			p->set_phase++;
			break;
		case PH1:
			if((myData->mData.misc.timer_1tick_count
				- myData->CAN.chip_diff_1tick_count[num]) < 10) { //10ms
				break;
			}

			error = CHIP_Reset_11(num, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d reset\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH2:
			error = CHIP_MCU_set_parameter(num, 0, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d set_param\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH3:
			error = CHIP_MCU_get_parameter(num, 1);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d get_param\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH4:
			error = CHIP_MCU_get_status(num, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d get_status\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH5:
			error = CHIP_EccEnable_11(num, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d eccEnable\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase = PH10;
			}
			break;
		case PH74:
			//Configure device
			CHIP_ConfigureObjectReset(num);
	
			if(p->set_param_crc == 0) p->config.IsoCrcEnable = 0;
			else p->config.IsoCrcEnable = 1; //ISO_CRC
			p->config.StoreInTEF = 0;

			error = CHIP_Configure_11(num, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d config\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_op_phase = PH0;
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH75:
			switch(p->chip_op_phase) {
				case PH0:
					//Setup TX FIFO
					CHIP_TransmitChannelConfigureObjectReset(num);

					p->chip_op_phase++;
					//skip break;
				case PH1:
					p->txConfig.FifoSize = 7;
					p->txConfig.PayLoadSize = CHIP_PLSIZE_64;
					p->txConfig.TxPriority = 1;
					error =
						CHIP_TransmitChannelConfigure_11(num, CHIP_FIFO_CH2, 0);
					if(error < 0) {
						printk(KERN_WARNING "rt_can_fd: param_11 error %d trChConfig\n", num);
						p->set_phase = PH100;
						break; //fail
					} else if(error == 0) {
						break; //wait
					} else {} //end

					//Setup RX FIFO
					CHIP_ReceiveChannelConfigureObjectReset(num);

					p->chip_transfer_phase = PH0;
					p->chip_op_phase++;
					break;
				case PH2:
					p->rxConfig.FifoSize = 15;
					p->rxConfig.PayLoadSize = CHIP_PLSIZE_64;
					error
						= CHIP_ReceiveChannelConfigure_11(num, CHIP_FIFO_CH1,0);
					if(error < 0) {
						printk(KERN_WARNING "rt_can_fd: param_11 error %d reChConfig\n", num);
						p->set_phase = PH100;
						break; //fail
					} else if(error == 0) {
						break; //wait
					} else { //end
						p->chip_op_phase = PH0;
						p->chip_transfer_phase = PH0;
						p->set_phase++;
					}
					break;
				default:
					break;
			}
			break;
		case PH76:
			error = rt_can_set_filter_1(num);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d filter1\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH77:
			error = rt_can_set_filter_2(num);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d filter2\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH78:
			error = rt_can_set_filter_3(num);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d filter3\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH79:
			error = rt_can_set_etc_1(num);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d etc1\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH80:
			error = rt_can_set_etc_2(num);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d etc2\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH81:
			error = rt_can_set_etc_3(num);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d etc3\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH82:
			error = CHIP_MCU_set_parameter(num, 1, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d set_param2\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH83:
			error = CHIP_MCU_get_parameter(num, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d get_param2\n", num);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				myData->CAN.chip_diff_1tick_count[num]
					= myData->mData.misc.timer_1tick_count;

				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
		case PH84:
			if((myData->mData.misc.timer_1tick_count
				- myData->CAN.chip_diff_1tick_count[num]) < 100) { //100ms
				break;
			}

			p->set_phase = PH100;

			p->chip_transfer_phase = PH0;
			p->chip_tx_phase = PH0;
			p->chip_rx_phase = PH0;
			printk(KERN_WARNING "rt_can_fd: set_param_11 end %d\n", num);
			rtn = 1;
			break;
		case PH100:
			if(p->tmp_set_phase == PH1) {
				p->set_phase = PH0;
			}
			break;
		default: //10~73
			error = CHIP_RamInit_11(num, 0);
			if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: param_11 error %d ramInit %d\n", num, p->set_phase);
				p->set_phase = PH100;
				break;
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->set_phase++;
			}
			break;
	}

	return rtn;
}

int rt_can_set_filter_1(int num)
{
	unsigned char log_flag = 0;
	int i, can_ch, can_type, error = 0;
	long filter_id, mask_id;
	T_CHIP_filter filter;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
			p->chip_filter_phase = PH0;
			p->chip_tmp_data_uc1 = 0;

			p->chip_op_phase++;
			//skip break;
		case PH1:
			i = (int)p->chip_tmp_data_uc1;
			//can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i];
			if(filter_id == 0) {
				//Link FIFO and Filter
				if(i == 0) filter = CHIP_FILT0;
				else filter = CHIP_FILT1;
				error = CHIP_FilterToFifoLink_11(num, filter, CHIP_FIFO_CH1,
					false, log_flag);
				if(error < 0) break; //fail
				else if(error == 0) {
					break; //wait
				} else { //end
					if(i == 0) {
						p->chip_tmp_data_uc1 = 1;
						p->chip_transfer_phase = PH0;
						error = 0;
						//kjg_240624 break;
					} else {
						p->chip_op_phase = PH0; //kjg_240624
					}

					if(log_ch != 0 && num == (log_ch - 1)) {
						printk(KERN_WARNING "rt_can_fd: %d filter_1a %d\n",
							num, i);
					}
				}
			} else {
				p->chip_op_phase = PH10;
				error = 0;
			}
			break;
		case PH10:
			p->chip_filter_phase++;

			i = (int)p->chip_tmp_data_uc1;
			//can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i];
			mask_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].mask[0];

			printk(KERN_WARNING
				"kjgd_191124a(%d) ch:%d, mask:%lx, filter:%lx\n",
				i, num, mask_id, filter_id);

			p->chip_transfer_phase = PH0;
			p->chip_op_phase++;
			//skip break;
		case PH11:
			i = (int)p->chip_tmp_data_uc1;
			//kjg_240624 can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i];
			mask_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].mask[0];

			//Setup RX Filter
			//kjgw_240624 if(p->set_param_ext == 0) {
			if(filter_id < 0x800) { //kjh_230105
				p->fObj.word = 0;
				p->fObj.bF.SID = filter_id;
				p->fObj.bF.SID11 = 0;
				p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
				p->fObj.bF.EID = 0;
			} else {
				p->fObj.word = 0;
				p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
				p->fObj.bF.SID11 = 0;
				p->fObj.bF.EXIDE = 1;
				p->fObj.bF.EID = filter_id & 0x3FFFF;
			}

			if(i == 0) filter = CHIP_FILT0;
			else filter = CHIP_FILT1;
			error = CHIP_FilterObjectConfigure_11(num, filter, log_flag);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH12:
			i = (int)p->chip_tmp_data_uc1;
			//can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i];
			mask_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].mask[0];

			//Setup RX Mask
			//kjgw_240624 if(p->set_param_ext == 0) {
			if(filter_id < 0x800) { //kjh_230105
				p->mObj.word = 0;
				p->mObj.bF.MSID = mask_id;
				p->mObj.bF.MSID11 = 0;
				//kjg_221109 p->mObj.bF.MIDE = 1; //Only allow standard frame
				p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
				p->mObj.bF.MEID = 0x00;
			} else {
				p->mObj.word = 0;
				p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
				p->mObj.bF.MSID11 = 0;
				p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
				p->mObj.bF.MEID = mask_id & 0x3FFFF;
			}

			if(i == 0) filter = CHIP_FILT0;
			else filter = CHIP_FILT1;
			error = CHIP_FilterMaskConfigure_11(num, filter, log_flag);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH13:
			i = (int)p->chip_tmp_data_uc1;
			//Link FIFO and Filter
			if(i == 0) filter = CHIP_FILT0;
			else filter = CHIP_FILT1;
			error = CHIP_FilterToFifoLink_11(num, filter, CHIP_FIFO_CH1,
				true, log_flag);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				if(i == 0) {
					p->chip_tmp_data_uc1 = 1;

					p->chip_transfer_phase = PH0;
					p->chip_op_phase = PH1;
					error = 0;
				} else {
					p->chip_op_phase = PH0; //kjg_240624
				}

				if(log_ch != 0 && num == (log_ch - 1)) {
					printk(KERN_WARNING "rt_can_fd: %d filter_1b %d\n", num, i);
				}
			}
			break;
		default:
			break;
	}

	return error;
}

int rt_can_set_filter_2(int num)
{
	int i, can_ch, can_type, error = 0;
	long filter_id, mask_id;
	T_CHIP_filter filter;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
			p->chip_tmp_data_uc1 = 0;

			p->chip_op_phase++;
			//skip break;
		case PH1:
			i = (int)p->chip_tmp_data_uc1;
			//can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i+2];
			if(filter_id == 0) {
				//Link FIFO and Filter
				if(i == 0) filter = CHIP_FILT2;
				else if(i == 1) filter = CHIP_FILT3;
				else if(i == 2) filter = CHIP_FILT4;
				else filter = CHIP_FILT5;
				error = CHIP_FilterToFifoLink_11(num, filter, CHIP_FIFO_CH1,
					false, 0);
				if(error < 0) {
					break; //fail
				} else if(error == 0) {
					break; //wait
				} else { //end
					p->chip_transfer_phase = PH0;

					if(i == 0) {
						p->chip_tmp_data_uc1 = 1;
						error = 0;
					} else if(i == 1) {
						p->chip_tmp_data_uc1 = 2;
						error = 0;
					} else if(i == 2) {
						p->chip_tmp_data_uc1 = 3;
						error = 0;
					} else {
						p->chip_op_phase = PH0;
					}

					if(log_ch != 0 && num == (log_ch - 1)) {
						printk(KERN_WARNING "rt_can_fd: %d filter_2a %d\n",
							num, i);
					}
				}
			} else {
				p->chip_op_phase = PH10;
				error = 0;
			}
			break;
		case PH10:
			p->chip_filter_phase++;

			i = (int)p->chip_tmp_data_uc1;
			//can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i+2];
			mask_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].mask[1];

			printk(KERN_WARNING
				"kjgd_191124b(%d) ch:%d, mask:%lx, filter:%lx\n",
				i, num, mask_id, filter_id);

			p->chip_transfer_phase = PH0;
			p->chip_op_phase++;
			//skip break;
		case PH11:
			i = (int)p->chip_tmp_data_uc1;
			//can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i+2];
			mask_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].mask[1];

			//Setup RX Filter
			//kjgw_240624if(p->set_param_ext == 0) {
			if(filter_id < 0x800) { //kjh_230105
				p->fObj.word = 0;
				p->fObj.bF.SID = filter_id;
				p->fObj.bF.SID11 = 0;
				p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
				p->fObj.bF.EID = 0;
			} else {
				p->fObj.word = 0;
				p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
				p->fObj.bF.SID11 = 0;
				p->fObj.bF.EXIDE = 1;
				p->fObj.bF.EID = filter_id & 0x3FFFF;
			}

			if(i == 0) filter = CHIP_FILT2;
			else if(i == 1) filter = CHIP_FILT3;
			else if(i == 2) filter = CHIP_FILT4;
			else filter = CHIP_FILT5;

			error = CHIP_FilterObjectConfigure_11(num, filter, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH12:
			i = (int)p->chip_tmp_data_uc1;
			//can_ch = num / 2;
			//can_type = num % 2;
			switch(myData->mData.config.division_CAN) {	//ktg_230612s
				case 1:
					can_ch = num;
					can_type = 0;
					break;
				default:
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
						case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
						case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
						case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 2;
							can_type = num % 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							can_ch = num / 2;
							if(can_ch == 1) can_ch = 3;
							can_type = num % 2;
							break;
						default:
							can_ch = num / 2;
							can_type = num % 2;
							break;
					}
					break;
			}	//ktg_230612e

			filter_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].filter[i+2];
			mask_id = myData->canReceiveSetData.commonData[can_ch]
				[can_type].mask[1];

			//Setup RX Mask
			//kjgw_240624 if(p->set_param_ext == 0) {
			if(filter_id < 0x800) { //kjh_230105
				p->mObj.word = 0;
				p->mObj.bF.MSID = mask_id;
				p->mObj.bF.MSID11 = 0;
				//kjg_221109 p->mObj.bF.MIDE = 1; //Only allow standard frame
				p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
				p->mObj.bF.MEID = 0x00;
			} else {
				p->mObj.word = 0;
				p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
				p->mObj.bF.MSID11 = 0;
				p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
				p->mObj.bF.MEID = mask_id & 0x3FFFF;
			}

			if(i == 0) filter = CHIP_FILT2;
			else if(i == 1) filter = CHIP_FILT3;
			else if(i == 2) filter = CHIP_FILT4;
			else filter = CHIP_FILT5;

			error = CHIP_FilterMaskConfigure_11(num, filter, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH13:
			i = (int)p->chip_tmp_data_uc1;

			//Link FIFO and Filter
			if(i == 0) filter = CHIP_FILT2;
			else if(i == 1) filter = CHIP_FILT3;
			else if(i == 2) filter = CHIP_FILT4;
			else filter = CHIP_FILT5;

			error = CHIP_FilterToFifoLink_11(num, filter, CHIP_FIFO_CH1,
				true, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;

				if(i == 0) {
					p->chip_tmp_data_uc1 = 1;
					p->chip_op_phase = PH1;
					error = 0;
				} else if(i == 1) {
					p->chip_tmp_data_uc1 = 2;
					p->chip_op_phase = PH1;
					error = 0;
				} else if(i == 2) {
					p->chip_tmp_data_uc1 = 3;
					p->chip_op_phase = PH1;
					error = 0;
				} else {
					p->chip_op_phase = PH0;
				}

				if(log_ch != 0 && num == (log_ch - 1)) {
					printk(KERN_WARNING "rt_can_fd: %d filter_2b %d\n", num, i);
				}
			}
			break;
		default:
			break;
	}

	return error;
}

int rt_can_set_filter_3(int num)
{
	int error = 0;
	long filter_id, mask_id;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
			if(p->chip_filter_phase != PH0) {
				//error = 0;
				error = 1;		//csk_240603
				break;
			}

			p->chip_transfer_phase = PH0;
			p->chip_op_phase++;
			//skip break;
		case PH1:
			mask_id = 0; filter_id = 0; //kjgw_240624

			//Setup RX Filter
			//kjgw_240624 if(p->set_param_ext == 0) { 
			if(filter_id < 0x800) { //kjh_230105
				p->fObj.word = 0;
				p->fObj.bF.SID = filter_id;
				p->fObj.bF.SID11 = 0;
				p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
				p->fObj.bF.EID = 0;
			} else {
				//Setup RX Filter
				p->fObj.word = 0;
				p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
				p->fObj.bF.SID11 = 0;
				p->fObj.bF.EXIDE = 1;
				p->fObj.bF.EID = filter_id & 0x3FFFF;
			}

			error = CHIP_FilterObjectConfigure_11(num, CHIP_FILT0, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				if(p->chip_transfer_phase == PH30) {
					printk(KERN_WARNING
						"kjgd_191124c ch:%d, mask:%lx, filter:%lx\n",
						num, mask_id, filter_id);
				}

				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH2:
			mask_id = 0; filter_id = 0;

			//Setup RX Mask
			//kjgw_240624 if(p->set_param_ext == 0) {
			if(filter_id < 0x800) { //kjh_230105
				p->mObj.word = 0;
				p->mObj.bF.MSID = mask_id;
				p->mObj.bF.MSID11 = 0;
				//kjg_221109 p->mObj.bF.MIDE = 1; //Only allow standard frame
				p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
				p->mObj.bF.MEID = 0x00;
			} else {
				p->mObj.word = 0;
				p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
				p->mObj.bF.MSID11 = 0;
				p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
				p->mObj.bF.MEID = mask_id & 0x3FFFF;
			}

			error = CHIP_FilterMaskConfigure_11(num, CHIP_FILT0, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH3:
			error = CHIP_FilterToFifoLink_11(num, CHIP_FILT0, CHIP_FIFO_CH1,
				true, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				if(log_ch != 0 && num == (log_ch - 1)) {
					printk(KERN_WARNING "rt_can_fd: %d filter_3\n", num);
				}

				p->chip_op_phase = PH0;
			}
			break;
		default:
			break;
	}

	return error;
}

int rt_can_set_etc_1(int num)
{
	int error = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
			//Setup Bit Time
			if(p->set_param_fd_flag == 0) {
				switch(p->set_param_baud) {
					case 125000: p->selectedBitTime = CHIP_125K_500K; break;
					case 250000: p->selectedBitTime = CHIP_250K_500K; break;
					case 500000: p->selectedBitTime = CHIP_500K_1M; break;
					case 1000000: p->selectedBitTime = CHIP_1000K_4M; break;
					default: p->selectedBitTime = CHIP_500K_1M; break;
				}
			} else {
				switch(p->set_param_baud) {
					case 125000: p->selectedBitTime = CHIP_125K_500K; break;
					case 250000:
						switch(p->set_param_data_rate) {
							case 0: //500K
								p->selectedBitTime = CHIP_250K_500K;
								break;
							case 1: //833K
								p->selectedBitTime = CHIP_250K_833K;
								break;
							case 2: //1M
								p->selectedBitTime = CHIP_250K_1M;
								break;
							case 3: //1M5
								p->selectedBitTime = CHIP_250K_1M5;
								break;
							case 4: //2M
								p->selectedBitTime = CHIP_250K_2M;
								break;
							case 5: //3M
								p->selectedBitTime = CHIP_250K_3M;
								break;
							case 6: //4M
								p->selectedBitTime = CHIP_250K_4M;
								break;
							default: //2M
								p->selectedBitTime = CHIP_250K_2M;
								break;
						}
						break;
					case 500000:
						switch(p->set_param_data_rate) {
							case 0:
							case 1:
							case 2:
							case 3:
								p->selectedBitTime = CHIP_500K_1M;
								break;
							case 4: //2M
								p->selectedBitTime = CHIP_500K_2M;
								break;
							case 5: //3M
								//p->selectedBitTime = CHIP_500K_3M;
								p->selectedBitTime = CHIP_500K_3M2;
								break;
							case 6: //4M
								p->selectedBitTime = CHIP_500K_4M;
								break;
							case 7: //3M2 kjg_220714
								p->selectedBitTime = CHIP_500K_3M2;
								break;
							default: //2M
								p->selectedBitTime = CHIP_500K_2M;
								break;
						}
						break;
					case 1000000: p->selectedBitTime = CHIP_1000K_4M; break;
					default: p->selectedBitTime = CHIP_500K_1M; break;
				}
			}

			p->chip_op_phase++;
			//skip break;
		case PH1:
			error = CHIP_BitTimeConfigureNominal40MHz_11(num, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH2:
			//skip break;
		case PH3:
			error
				= CHIP_BitTimeConfigureData40MHz_11(num, CHIP_SSP_MODE_AUTO, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_op_phase = PH0;
			}
			break;
		default:
			break;
	}

	return error;
}

int rt_can_set_etc_2(int num)
{
	int error = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
			//skip break;
		case PH1:
			//Setup Transmit and Receive Interrupts
			error = CHIP_GpioModeConfigure_11(num, CHIP_PINMODE_INT,
				CHIP_PINMODE_INT, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase = PH3;
				error = 0;
			}
			break;
		case PH3:
			//skip break;
		case PH4:
			//error = CHIP_TransmitChannelEventEnable_11(num, CHIP_FIFO_CH2, CHIP_TX_FIFO_NOT_FULL_EVENT, 0);
			error = CHIP_TransmitChannelEventDisable_11(num, CHIP_FIFO_CH2, CHIP_TX_FIFO_NOT_FULL_EVENT, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase = PH6;
				error = 0;
			}
			break;
		case PH6:
			//skip break;
		case PH7:
			//skip break;
		case PH8:
			//skip break;
		case PH9:
			error = CHIP_ReceiveChannelEventEnable_11(num, CHIP_FIFO_CH1, CHIP_RX_FIFO_NOT_EMPTY_EVENT, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_op_phase = PH0;
			}
			break;
		default:
			break;
	}

	return error;
}

int rt_can_set_etc_3(int num)
{
	int error = 0;
	T_CHIP_opMode opMode;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
			//skip break;
		case PH1:
			//error = CHIP_ModuleEventEnable_11(num, CHIP_TX_EVENT || CHIP_RX_EVENT, 0);
			error = CHIP_ModuleEventEnable_11(num, CHIP_RX_EVENT, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase = PH3;
				error = 0;
			}
			break;
		case PH3:
			//skip break;
		case PH4:
			//Select Normal Mode
			if(p->set_param_fd_flag == 0) {
				p->flag = RT_CAN_FD_CLASSIC_FLAG;
				opMode = CHIP_CLASSIC_MODE;
			} else {
				p->flag = RT_CAN_FD_NORMAL_FLAG;
				opMode = CHIP_NORMAL_MODE;
			}

			error = CHIP_OperationModeSelect_11(num, opMode, 0);
			if(error < 0) { //fail
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_op_phase = PH0;
			}
			break;
		default:
			break;
	}

	return error;
}

int rt_can_check_status(int num)
{
	int status;

	if(rt_can_fd_table[num].tmp_set_phase == PH0) {
		status = (int)rt_can_fd_table[num].set_phase;
	} else {
		status = (int)rt_can_fd_table[num].tmp_set_phase;
	}

	return status;
}

int SPI_Transfer_11(int num, struct rt_can_fd_struct *p, unsigned char log_flag)
{
	unsigned char tx_len, rx_len, cmd, spi_cmd, i, fifo_status, spi_tx_index, fifo_rx_wr_index, spi_rx_length, spi_running_flag;
	int error = 0;
	unsigned int addr, addr2;

	//kjg_240503
	unsigned char uc_tmp, checksum_enable, checksum_h, checksum_l;
	unsigned int checksum_val, ui_tmp;
	checksum_enable = 1; //0:checksum unuse, 1:checksum use
	checksum_val = 0;

	addr = RT_CAN_FD_BASE_ADDR + 0x40 + (0x10 * num);

	tx_len = p->chip_transfer_tx_length;
	rx_len = p->chip_transfer_rx_length;
	cmd = p->chip_transfer_cmd;

	switch(p->chip_transfer_phase) {
		case PH1:
			fifo_status = inb(addr + 3) & 0x0F;
			if(fifo_status != 0x0A) {
				if(p->error_print_flag == 0) {
					p->error_print_flag = 1;
					printk(KERN_WARNING
						"rt_can_fd: SPI_Transfer_11 error1 %d %d %d %d %02x\n",
						num, tx_len, rx_len, cmd, fifo_status);
				}
				myData->test_val_i[num][15]++;
				outb(0x00, addr); //clear
				error = (-1);
				return error;
			}

			p->error_print_flag = 0;

			outb(0x00, addr); //clear

			uc_tmp = 3 + tx_len;
			outb(uc_tmp, addr+1);
			checksum_val += uc_tmp;
			uc_tmp = 1 + rx_len;
			outb(uc_tmp, addr+2);
			checksum_val += uc_tmp;

			if(cmd <= 100) spi_cmd = 0x00;
			else spi_cmd = (unsigned char)(cmd % 100);
			outb(spi_cmd, addr+3);
			checksum_val += spi_cmd;

			if(tx_len != 0) {
				for(i=0; i < tx_len; i++) {
					outb(p->spi_tx_buf[i], addr+4);
					checksum_val += p->spi_tx_buf[i];
				}
			}

			if(checksum_enable == 1) {
				checksum_h = ((checksum_val & 0x0000FF00) >> 8);
				checksum_l = (checksum_val & 0x000000FF);
				outb(checksum_h, addr+6);
				outb(checksum_l, addr+7);
			}

			outb(0x00, addr+5); //spi start

			p->chip_transfer_count = 0;
			p->chip_transfer_phase++;
			//break;
		case PH2:
			i = 0;
			while(1) {
				spi_tx_index = inb(addr);
				fifo_rx_wr_index = inb(addr+2);
				fifo_status = inb(addr+3) & 0x03;
				spi_rx_length = inb(addr+4);

				//if(num < 4) spi_running_flag = (inb(0x805) & (0x01 << num));
				//else spi_running_flag = (inb(0x825) & (0x01 << (num - 4)));
				if(num < 4) addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_IN_3;
				else addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_IN_3;
				uc_tmp = inb(addr2);
				spi_running_flag = uc_tmp & (0x01 << (num % 4));

				if(spi_tx_index == (3 + tx_len + 1 + rx_len)
					&& spi_tx_index == fifo_rx_wr_index
					&& fifo_status == 0x02
					&& spi_rx_length == (1+rx_len)
					&& spi_running_flag == 0x00) break;

				i++;
				//kjg_240624 if(i > 3) {
				if(i >= 1) {
					p->chip_transfer_count++;
					if(p->chip_transfer_count < 10) error = 0;
					else {
						error = (-2);
						if(p->error_print_flag == 0) {
							p->error_print_flag = 1;
							printk(KERN_WARNING
								"rt_can_fd: SPI_Transfer_11 error2 ch%d cmd%d %d %d, %d %d %02x %d, flag%02x %d\n",
								num, cmd, tx_len, rx_len, spi_tx_index, fifo_rx_wr_index, fifo_status, spi_rx_length, spi_running_flag, p->chip_transfer_count);
						}
						myData->test_val_i[num][15]++;

						outb(0x00, addr); //clear
					}
					return error;
				}
			}

			p->error_print_flag = 0;

			p->chip_transfer_count = 0;
			p->chip_transfer_phase++;
			//kjg_240217 skip break;
			//break;
		case PH3:
			checksum_val = 0;
			//kjg_240223 if(rx_len != 0) {
				for(i=0; i < (4 + tx_len); i++) checksum_val += inb(addr+1);
				for(i=0; i < rx_len; i++) {
					p->spi_rx_buf[i] = inb(addr+1);
					checksum_val += p->spi_rx_buf[i];
				}
			//}

			if(checksum_enable == 1) {
				checksum_h = inb(addr+5);
				checksum_l = inb(addr+6);
				ui_tmp = ((unsigned int)checksum_h << 8) + checksum_l;
				if(checksum_val != ui_tmp) {
					outb(0x00, addr); //clear

					p->chip_transfer_count = 0;
					error = (-10);
					if(p->error_print_flag == 0) {
						p->error_print_flag = 1;
						printk(KERN_WARNING
							"rt_can_fd: SPI_Transfer_11 error3 %d %d, %d %d, %04x %02x %02x\n",
							num, cmd, tx_len, rx_len,
							checksum_val, checksum_h, checksum_l);
					}
					myData->test_val_i[num][15]++;
					break;
				}
			}

			fifo_status = inb(addr+3) & 0x0F;
			if(fifo_status == 0x0A) {
				outb(0x00, addr); //clear
	
				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING
						"rt_can_fd: %d SPI_Transfer_11b %d %d %d\n",
						num, cmd, tx_len, rx_len);
				}
	
				p->chip_transfer_phase = PH10;
				error = 1;
				break; //kjg_240624
			} else {
				p->chip_transfer_count = 0;
				p->chip_transfer_phase++;
			}
			//kjg_240223 break;
		case PH4:
			fifo_status = inb(addr+3) & 0x0F;
			if(fifo_status == 0x0A) {
				outb(0x00, addr); //clear

				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING
						"rt_can_fd: %d SPI_Transfer_11d %d %d %d\n",
						num, cmd, tx_len, rx_len);
				}
	
				p->chip_transfer_phase = PH10;
				error = 1;
			} else {
				p->chip_transfer_count++;
				if(p->chip_transfer_count < 10) {
					error = 0;
				} else {
					outb(0x00, addr); //clear

					p->chip_transfer_count = 0;
					error = (-10);
					if(p->error_print_flag == 0) {
						p->error_print_flag = 1;
						printk(KERN_WARNING
							"rt_can_fd: SPI_Transfer_11 error10 %d %d %02x, %d %d\n",
							num, cmd, fifo_status, tx_len, rx_len);
					}
					myData->test_val_i[num][15]++;
				}
			}
			break;
		default:
			break;
	}

	return error;
}

int SPI_Transfer_TX(int num, struct rt_can_fd_struct *p, unsigned char log_flag)
{
	unsigned char tx_len, rx_len, cmd, spi_cmd, i, fifo_status, spi_tx_index, fifo_rx_wr_index, spi_rx_length, tx_only = 0, spi_running_flag;
	int error = 0;
	unsigned int addr, addr2;

	//kjg_240503
	unsigned char uc_tmp, checksum_enable, checksum_h, checksum_l;
	unsigned int checksum_val, ui_tmp;
	checksum_enable = 1; //0:checksum unuse, 1:checksum use
	checksum_val = 0;

	addr = RT_CAN_FD_BASE_ADDR + 0x40 + (0x10 * num);

	tx_len = p->chip_transfer_tx_length;
	rx_len = p->chip_transfer_rx_length;
	cmd = p->chip_transfer_cmd;

	switch(p->chip_transfer_phase) {
		case PH1:
			fifo_status = inb(addr + 3) & 0x0F;
			if(fifo_status != 0x0A) {
				if(p->error_print_flag == 0) {
					p->error_print_flag = 1;
					printk(KERN_WARNING
						"rt_can_fd: SPI_Transfer_TX error1 %d %d %d %d %02x\n",
						num, tx_len, rx_len, cmd, fifo_status);
				}
				myData->test_val_i[num][15]++;
				outb(0x00, addr); //clear
				error = (-1);
				return error;
			}

			p->error_print_flag = 0;

			outb(0x00, addr); //clear

			uc_tmp = 3 + tx_len;
			outb(uc_tmp, addr+1);
			checksum_val += uc_tmp;
			uc_tmp = 1 + rx_len;
			outb(uc_tmp, addr+2);
			checksum_val += uc_tmp;

			if(cmd <= 100) spi_cmd = 0x00;
			else spi_cmd = (unsigned char)(cmd % 100);
			outb(spi_cmd, addr+3);
			checksum_val += spi_cmd;

			if(tx_len != 0) {
				for(i=0; i < tx_len; i++) {
					outb(p->spi_tx_buf[i], addr+4);
					checksum_val += p->spi_tx_buf[i];
				}
			}

			if(checksum_enable == 1) {
				checksum_h = ((checksum_val & 0x0000FF00) >> 8);
				checksum_l = (checksum_val & 0x000000FF);
				outb(checksum_h, addr+6);
				outb(checksum_l, addr+7);
			}

			outb(0x00, addr+5); //spi start

			p->chip_transfer_count = 0;

			if(tx_only == 1) { //kjgw_240222
				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING
						"rt_can_fd: %d SPI_Transfer_TX_a %d %d %d\n",
						num, cmd, tx_len, rx_len);
				}

				p->error_print_flag = 0;
				p->chip_transfer_phase = PH10;
				outb(0x00, addr); //clear
				error = 2;
				break;
			}

			p->chip_transfer_phase++;
			//kjg_240217 skip break;
			//break;
		case PH2:
			i = 0;
			while(1) {
				spi_tx_index = inb(addr);
				fifo_rx_wr_index = inb(addr+2);
				fifo_status = inb(addr+3) & 0x03;
				spi_rx_length = inb(addr+4);

				//if(num < 4) spi_running_flag = (inb(0x805) & (0x01 << num));
				//else spi_running_flag = (inb(0x825) & (0x01 << (num - 4)));
				if(num < 4) addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_IN_3;
				else addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_IN_3;
				uc_tmp = inb(addr2);
				spi_running_flag = uc_tmp & (0x01 << (num % 4));

				if(spi_tx_index == (3 + tx_len + 1 + rx_len)
					&& spi_tx_index == fifo_rx_wr_index
					&& fifo_status == 0x02
					&& spi_rx_length == (1+rx_len)
					&& spi_running_flag == 0x00) break;

				i++;
				if(i > 3) {
					p->chip_transfer_count++;
					if(p->chip_transfer_count < 10) error = 0;
					else {
						error = (-2);
						if(p->error_print_flag == 0) {
							p->error_print_flag = 1;
							printk(KERN_WARNING
								"rt_can_fd: SPI_Transfer_TX error2 ch%d cmd%d %d %d, %d %d %02x %d, flag%02x %d\n",
								num, cmd, tx_len, rx_len, spi_tx_index, fifo_rx_wr_index, fifo_status, spi_rx_length, spi_running_flag, p->chip_transfer_count);
						}
						myData->test_val_i[num][15]++;

						outb(0x00, addr); //clear
					}
					return error;
				}
			}

			p->error_print_flag = 0;

			p->chip_transfer_count = 0;
			p->chip_transfer_phase++;

			outb(0x00, addr); //clear

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: %d SPI_Transfer_TX_d %d %d %d\n",
					num, cmd, tx_len, rx_len);
			}

			p->chip_transfer_phase = PH10;
			error = 1;
			break;

			//kjg_240217 skip break;
			//break;
		case PH3:
			checksum_val = 0;
			if(rx_len != 0) {
				for(i=0; i < (4 + tx_len); i++) checksum_val += inb(addr+1);
				for(i=0; i < rx_len; i++) {
					p->spi_rx_buf[i] = inb(addr+1);
					checksum_val += p->spi_rx_buf[i];
				}
			}

			if(checksum_enable == 1) {
				checksum_h = inb(addr+5);
				checksum_l = inb(addr+6);
				ui_tmp = ((unsigned int)checksum_h << 8) + checksum_l;
				if(checksum_val != ui_tmp) {
					outb(0x00, addr); //clear

					p->chip_transfer_count = 0;
					error = (-10);
					if(p->error_print_flag == 0) {
						p->error_print_flag = 1;
						printk(KERN_WARNING
							"rt_can_fd: SPI_Transfer_TX error3 %d %d, %d %d, %04x %02x %02x\n",
							num, cmd, tx_len, rx_len,
							checksum_val, checksum_h, checksum_l);
					}
					myData->test_val_i[num][15]++;
					break;
				}
			}

			fifo_status = inb(addr+3) & 0x0F;
			if(fifo_status == 0x0A) {
				outb(0x00, addr); //clear

				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING
						"rt_can_fd: %d SPI_Transfer_TX_b %d %d %d\n",
						num, cmd, tx_len, rx_len);
				}

				p->chip_transfer_phase = PH10;
				error = 1;
			} else if(fifo_status == 0x02 && rx_len == 0 && cmd == 112) {
				outb(0x00, addr); //clear

				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING
						"rt_can_fd: %d SPI_Transfer_TX_c %d %d %d\n",
						num, cmd, tx_len, rx_len);
				}

				p->chip_transfer_phase = PH10;
				error = 1;
			} else {
				p->chip_transfer_count = 0;
				p->chip_transfer_phase++;
			}
			//skip break;
		case PH4:
			fifo_status = inb(addr+3) % 0x0F;
			if(fifo_status == 0x0A) {
				outb(0x00, addr); //clear

				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING
						"rt_can_fd: %d SPI_Transfer_TX_d %d %d %d\n",
						num, cmd, tx_len, rx_len);
				}

				p->chip_transfer_phase = PH10;
				error = 1;
			} else {
				p->chip_transfer_count++;
				if(p->chip_transfer_count < 10) {
					error = 0;
				} else {
					outb(0x00, addr); //clear

					p->chip_transfer_count = 0;
					error = (-10);
					if(p->error_print_flag == 0) {
						p->error_print_flag = 1;
						printk(KERN_WARNING
							"rt_can_fd: SPI_Transfer_TX error10 %d %d %02x, %d %d\n", num, cmd, fifo_status, tx_len, rx_len);
					}
					myData->test_val_i[num][15]++;
				}
			}
			break;
		default:
			break;
	}

	return error;
}

int SPI_Transfer_RX(int num, struct rt_can_fd_struct *p, unsigned char log_flag)
{
	unsigned char tx_len, rx_len, cmd, spi_cmd, i, fifo_status, spi_tx_index, fifo_rx_wr_index, spi_rx_length, spi_running_flag;
	int error = 0;
	unsigned int addr, addr2;

	//kjg_240503
	unsigned char uc_tmp, checksum_enable, checksum_h, checksum_l;
	unsigned int checksum_val, ui_tmp;
	checksum_enable = 1; //0:checksum unuse, 1:checksum use
	checksum_val = 0;

	addr = RT_CAN_FD_BASE_ADDR + 0x40 + (0x10 * num);

	tx_len = p->chip_transfer_tx_length;
	rx_len = p->chip_transfer_rx_length;
	cmd = p->chip_transfer_cmd;

	switch(p->chip_transfer_phase) {
		case PH1:
			fifo_status = inb(addr + 3) & 0x0F;
			if(fifo_status != 0x0A) {
				if(p->error_print_flag == 0) {
					p->error_print_flag = 1;
					printk(KERN_WARNING
						"rt_can_fd: SPI_Transfer_RX error1 %d %d %d %d %02x\n",
						num, tx_len, rx_len, cmd, fifo_status);
				}
				myData->test_val_i[num][15]++;
				outb(0x00, addr); //clear
				error = (-1);
				return error;
			}

			p->error_print_flag = 0;

			outb(0x00, addr); //clear

			uc_tmp = 3 + tx_len;
			outb(uc_tmp, addr+1);
			checksum_val += uc_tmp;
			uc_tmp = 1 + rx_len;
			outb(uc_tmp, addr+2);
			checksum_val += uc_tmp;

			if(cmd <= 100) spi_cmd = 0x00;
			else spi_cmd = (unsigned char)(cmd % 100);
			outb(spi_cmd, addr+3);
			checksum_val += spi_cmd;

			if(tx_len != 0) {
				for(i=0; i < tx_len; i++) {
					outb(p->spi_tx_buf[i], addr+4);
					checksum_val += p->spi_tx_buf[i];
				}
			}

			if(checksum_enable == 1) {
				checksum_h = ((checksum_val & 0x0000FF00) >> 8);
				checksum_l = (checksum_val & 0x000000FF);
				outb(checksum_h, addr+6);
				outb(checksum_l, addr+7);
			}

			outb(0x00, addr+5); //spi start

			p->chip_transfer_count = 0;
			p->chip_transfer_phase++;
			//kjg_240217 skip break;
			//break;
		case PH2:
			i = 0;
			while(1) {
				spi_tx_index = inb(addr);
				fifo_rx_wr_index = inb(addr+2);
				fifo_status = inb(addr+3) & 0x03;

				if(spi_tx_index >= (3 + tx_len + 1 + rx_len)
					&& fifo_status == 0x02 && fifo_rx_wr_index >= 14) break;

				i++;
				//kjg_240624 if(i > 3) { 
				if(i >= 1) {
					p->chip_transfer_count++;
					if(p->chip_transfer_count < 10) error = 0;
					else {
						error = (-2);
						if(p->error_print_flag == 0) {
							p->error_print_flag = 1;
							printk(KERN_WARNING
							"rt_can_fd: SPI_Transfer_RX error2 ch%d cmd%d %d %d, %d %d %02x\n",
							num, cmd, tx_len, rx_len, spi_tx_index, fifo_rx_wr_index, fifo_status);
						}
						myData->test_val_i[num][15]++;

						outb(0x00, addr); //clear
					}
					return error;
				}
			}

			p->error_print_flag = 0;

			checksum_val = 0;
			for(i=0; i < 4; i++) checksum_val += inb(addr + 1);
			p->chip_transfer_received_length = inb(addr + 1);
			checksum_val += p->chip_transfer_received_length;
			p->chip_tmp_data_ui1 = checksum_val;

			p->chip_transfer_count = 0;
			p->chip_transfer_phase++;
			//kjg_240217 skip break;
			//break;
		case PH3:
			i = 0;
			while(1) {
				fifo_rx_wr_index = inb(addr+2);
				spi_rx_length = inb(addr+4);

				spi_tx_index = inb(addr);
				fifo_status = inb(addr+3) & 0x03;
				if(num < 4) addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_IN_3;
				else addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_IN_3;
				uc_tmp = inb(addr2);
				spi_running_flag = uc_tmp & (0x01 << (num % 4));

				if(fifo_rx_wr_index == (4 + p->chip_transfer_received_length)
					&& spi_rx_length == (1 + p->chip_transfer_received_length)
					&& spi_running_flag == 0x00)
					break;

				i++;
				//kjg_240624 if(i > 3) {
				if(i >= 1) {
					p->chip_transfer_count++;
					if(p->chip_transfer_count < 10) error = 0;
					else {
						error = (-3);
						if(p->error_print_flag == 0) {
							p->error_print_flag = 1;
							printk(KERN_WARNING
								"rt_can_fd: SPI_Transfer_RX error3 ch%d cmd%d %d %d, %d %d %02x %d, flag%02x %d\n",
								num, cmd, tx_len, rx_len, spi_tx_index,
								fifo_rx_wr_index, fifo_status, spi_rx_length,
								spi_running_flag, p->chip_transfer_count);
						}

						outb(0x00, addr); //clear
					}
					return error;
				}
			}

			p->error_print_flag = 0;

			checksum_val = p->chip_tmp_data_ui1;
			for(i=5; i < (p->chip_transfer_received_length + 4); i++) {
				p->spi_rx_buf[i - 5] = inb(addr+1);
				checksum_val += p->spi_rx_buf[i - 5];
			}

			if(checksum_enable == 1) {
				checksum_h = inb(addr+5);
				checksum_l = inb(addr+6);
				ui_tmp = ((unsigned int)checksum_h << 8) + checksum_l;
				if(checksum_val != ui_tmp) {
					outb(0x00, addr); //clear

					p->chip_transfer_count = 0;
					error = (-3);
					if(p->error_print_flag == 0) {
						p->error_print_flag = 1;
						printk(KERN_WARNING
							"rt_can_fd: SPI_Transfer_RX error5 %d %d, %d %d %d, %04x %02x %02x\n",
							num, cmd, fifo_rx_wr_index, spi_rx_length, p->chip_transfer_received_length, checksum_val, checksum_h, checksum_l);
					}
					myData->test_val_i[num][15]++;
					return error;
				}
			}

			outb(0x00, addr); //clear

			p->chip_transfer_phase = PH10;

			if(p->chip_transfer_received_length == 10) {
				if(p->error_print_flag == 0) {
					p->error_print_flag = 1;
					printk(KERN_WARNING
						"rt_can_fd: SPI_Transfer_RX error4 %d %d, %d %d %d\n",
						num, cmd, fifo_rx_wr_index, spi_rx_length, p->chip_transfer_received_length);
				}
				myData->test_val_i[num][15]++;
				//kjg_240217 error = (-4);
				error = 2;
				return error;
			}

			p->error_print_flag = 0;

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
			printk(KERN_WARNING
				"rt_can_fd: SPI_Transfer_RX value %d %d, %d %d, %d\n",
				num, cmd, fifo_rx_wr_index, spi_rx_length, p->chip_transfer_received_length);
			printk(KERN_WARNING
				"rt_can_fd: %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x, %x %x %x %x %x\n",
				p->spi_rx_buf[0], p->spi_rx_buf[1], p->spi_rx_buf[2], p->spi_rx_buf[3], p->spi_rx_buf[4],
				p->spi_rx_buf[5], p->spi_rx_buf[6], p->spi_rx_buf[7], p->spi_rx_buf[8], p->spi_rx_buf[9],
				p->spi_rx_buf[10], p->spi_rx_buf[11], p->spi_rx_buf[12], p->spi_rx_buf[13], p->spi_rx_buf[14],
				p->spi_rx_buf[15], p->spi_rx_buf[16], p->spi_rx_buf[17], p->spi_rx_buf[18], p->spi_rx_buf[19]);
			}

			error = 1;
			break;
		default:
			break;
	}

	return error;
}

//Public Function Definitions
char CHIP_MCU_set_parameter(int num, unsigned char can_enable, unsigned char log_flag)
{
    char error = 0;
	U_US_DATA u_us_data1;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 4;
			p->chip_transfer_rx_length = 0;
			p->chip_transfer_cmd = 102;

			u_us_data1.val = 40; //40ms kjg_240624 80ms -> 40ms
			p->spi_tx_buf[0] = u_us_data1.byte[0];
			p->spi_tx_buf[1] = u_us_data1.byte[1];

			if(p->set_param_fd_flag == 0) {
				p->spi_tx_buf[2] = 0;
			} else {
				p->spi_tx_buf[2] = 1;
			}
			p->spi_tx_buf[3] = can_enable; //0:disable, 1:enable

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
			error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: %d MCU_set_parameter\n", num);
			}
			break;
		default:
			break;
	}

    return error;
}

char CHIP_MCU_get_parameter(int num, unsigned char log_flag)
{
    char error = 0, model_name[17], firmware_ver[9];
	unsigned char can_fd_flag;
	unsigned short filter_time;
	U_US_DATA u_us_data1;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 0;
			p->chip_transfer_rx_length = 44;
			p->chip_transfer_cmd = 101;

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
			error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

			memcpy((char *)&model_name[0], (char *)&p->spi_rx_buf[0], 16);
			model_name[16] = 0;
			memcpy((char *)&firmware_ver[0], (char *)&p->spi_rx_buf[32], 8);
			firmware_ver[8] = 0;
			u_us_data1.byte[0] = p->spi_rx_buf[40];
			u_us_data1.byte[1] = p->spi_rx_buf[41];
			filter_time = u_us_data1.val;

			can_fd_flag = p->spi_rx_buf[42];

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: %d %s ver:%s f_t %d, fd_flag %d\n",
					num, model_name, firmware_ver, filter_time, can_fd_flag);
			}
			break;
		default:
			break;
	}

    return error;
}

char CHIP_MCU_get_status(int num, unsigned char log_flag)
{
    char error = 0;
	U_US_DATA u_us_data1;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 0;
			//kjg_240624 p->chip_transfer_rx_length = 6;
			p->chip_transfer_rx_length = 16;
			p->chip_transfer_cmd = 103;

			p->spi_tx_buf[0] = 0;

			if(num == 0) {
				myData->test_val_l2[1] = 1;
				myData->test_val_l2[2] = 0;
			}
			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
			error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

			u_us_data1.byte[0] = p->spi_rx_buf[0];
			u_us_data1.byte[1] = p->spi_rx_buf[1];
			p->mcu_filter_id_count = u_us_data1.val;

			p->mcu_chip_op_phase = p->spi_rx_buf[2];
			p->mcu_rx_msg_count = p->spi_rx_buf[3];
			p->mcu_rx_msg_wr_idx = p->spi_rx_buf[4];
			p->mcu_rx_msg_rd_idx = p->spi_rx_buf[5];

			if(num == 0) myData->test_val_l2[0] = (long)p->mcu_rx_msg_count;

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING "rt_can_fd: CHIP_MCU_get_status %d, id_count %d, op_phase %d, msg_count %d, wr_idx %d, rd_idx %d\n",
					num, p->mcu_filter_id_count, p->mcu_chip_op_phase,
					p->mcu_rx_msg_count, p->mcu_rx_msg_wr_idx,
					p->mcu_rx_msg_rd_idx);
			}
			break;
		default:
			break;
	}

    return error;
}

char CHIP_MCU_ReceiveMessageGet(int num)
{
    char error = 0;
    unsigned short i;
    unsigned int idx;
	U_UI_DATA u_ui_data1;
    struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 0;
			p->chip_transfer_rx_length = 10;
			p->chip_transfer_cmd = 111;

			p->spi_tx_buf[0] = 0;
			p->spi_tx_buf[1] = 0;

			if(num == 0) {
				myData->test_val_l2[4] = 1;
				myData->test_val_l2[5] = 0;
			}

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
			error = SPI_Transfer_RX(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

			if(p->spi_rx_buf[0] == 0) { //mcu_rx_msg_count
				printk(KERN_WARNING
					"rt_can_fd: CHIP_MCU_ReceiveMessageGet error1 %d, id_count %d, op_phase %d, msg_count %d, wr_idx %d, rd_idx %d\n",
					num, p->mcu_filter_id_count, p->mcu_chip_op_phase,
					p->spi_rx_buf[0], p->mcu_rx_msg_wr_idx,
					p->mcu_rx_msg_rd_idx);

				error = 101;
				break;
			}

			u_ui_data1.byte[0] = p->spi_rx_buf[1];
			u_ui_data1.byte[1] = p->spi_rx_buf[2];
			u_ui_data1.byte[2] = p->spi_rx_buf[3];
			u_ui_data1.byte[3] = p->spi_rx_buf[4];
			p->tmp_msg.id = u_ui_data1.val;
			if(p->tmp_msg.id == 0) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_MCU_ReceiveMessageGet error2 %d, id_count %d, op_phase %d, msg_count %d, wr_idx %d, rd_idx %d\n",
					num, p->mcu_filter_id_count, p->mcu_chip_op_phase,
					p->spi_rx_buf[0], p->mcu_rx_msg_wr_idx,
					p->mcu_rx_msg_rd_idx);

				error = 102;
				break;
			}

			p->tmp_msg.type = p->spi_rx_buf[5];
			p->tmp_msg.length = p->spi_rx_buf[6];

			for(i=0; i < p->tmp_msg.length; i++)
				p->tmp_msg.data[i] = p->spi_rx_buf[i+9];

			if(p->rx_msg_count >= RT_CAN_FD_RX_MSG_BUF_SIZE) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_MCU_ReceiveMessageGet error3 %d, id_count %d, op_phase %d, msg_count %d, wr_idx %d, rd_idx %d\n",
					num, p->mcu_filter_id_count, p->mcu_chip_op_phase,
					p->spi_rx_buf[0], p->mcu_rx_msg_wr_idx,
					p->mcu_rx_msg_rd_idx);

				error = 103;
				//kjgw_240209 p->chip_transfer_phase = PH20;
				break;
			}

			idx = p->rx_msg_write_idx;
			idx++;
			if(idx >= RT_CAN_FD_RX_MSG_BUF_SIZE) idx = 0;

			p->mcu_rx_msg_count = p->spi_rx_buf[0]; //kjg_240223

			p->rx_buf[idx].id = p->tmp_msg.id;
			p->rx_buf[idx].type = p->tmp_msg.type;
			p->rx_buf[idx].length = p->tmp_msg.length;
			for(i=0; i < p->tmp_msg.length; i++)
				p->rx_buf[idx].data[i] = p->tmp_msg.data[i];
			p->rx_buf[idx].time = p->tmp_msg.time;
			p->rx_buf[idx].timeStamp = p->tmp_msg.timeStamp;

			p->rx_msg_write_idx = idx;
			p->rx_msg_count++;

			error = 100;
			break;
		default:
			break;
	}

    return error;
}

char CHIP_Reset_11(int num, unsigned char log_flag)
{
	char error = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 2;
			p->chip_transfer_rx_length = 0;
			p->chip_transfer_cmd = 1;
			
			p->spi_tx_buf[0] = (unsigned char)(_CHIP_INS_RESET << 4);
			p->spi_tx_buf[1] = 0;

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
			error = SPI_Transfer_11(num, p, 0);
			if(error < 0) {
				p->chip_transfer_phase = PH20; //fail
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING "rt_can_fd: %d Reset_11\n", num);
			}
			break;
		default:
			break;
	}

	return error;
}

char CHIP_ReadByte_11(int num, unsigned short address, unsigned char *rxd)
{
    char error = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 2;
			p->chip_transfer_rx_length = 1;
			p->chip_transfer_cmd = 2;

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ << 4)
				+ ((address >> 8) & 0xF));
		    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

		    *rxd = p->spi_rx_buf[0]; //Update data
			break;
		default:
			break;
	}

    return error;
}

char CHIP_WriteByte_11(int num, unsigned short address, unsigned char txd)
{
    char error = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 3;
			p->chip_transfer_rx_length = 0;
			p->chip_transfer_cmd = 3;

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
				+ ((address >> 8) & 0xF));
		    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
		    p->spi_tx_buf[2] = txd;

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, 0);
			if(error < 0) {
				p->chip_transfer_phase = PH20; //fail
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end
			break;
		default:
			break;
	}

    return error;
}

char CHIP_ReadWord_11(int num, unsigned short address, unsigned int *rxd)
{
    char error = 0;
    unsigned char i;
    unsigned int x;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 2;
			p->chip_transfer_rx_length = 4;
			p->chip_transfer_cmd = 4;

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ << 4)
				+ ((address >> 8) & 0xF));
		    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

		    //Update data
		    *rxd = 0;
		    for(i=2; i < 6; i++) {
		        x = (unsigned int)p->spi_rx_buf[i];
 		       *rxd += x << ((i - 2) * 8);
 			}
			break;
		default:
			break;
	}

    return error;
}

char CHIP_WriteWord_11(int num, unsigned short address, unsigned int txd)
{
    char error = 0;
    unsigned char i, log_flag = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 6;
			p->chip_transfer_rx_length = 0;
			p->chip_transfer_cmd = 5;

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
				+ ((address >> 8) & 0xF));
		    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

  			//Split word into 4 bytes and add them to buffer
			for(i=0; i < 4; i++) {
		        p->spi_tx_buf[i + 2] = (unsigned char)((txd >> (i * 8)) & 0xFF);
		    }

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, log_flag);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end
			break;
		default:
			break;
	}

    return error;
}

char CHIP_ReadHalfWord_11(int num, unsigned short address, unsigned short *rxd)
{
    char error = 0;
    unsigned char i;
    unsigned int x;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 2;
			p->chip_transfer_rx_length = 2;
			p->chip_transfer_cmd = 6;

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ << 4)
				+ ((address >> 8) & 0xF));
  			p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

		    //Update data
		    *rxd = 0;
		    for(i=2; i < 4; i++) {
		        x = (unsigned int)p->spi_rx_buf[i];
		        *rxd += x << ((i - 2) * 8);
 			}
			break;
		default:
			break;
	}

    return error;
}

char CHIP_WriteHalfWord_11(int num, unsigned short address, unsigned short txd)
{
    char error = 0;
    unsigned char i;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 4;
			p->chip_transfer_rx_length = 0;
			p->chip_transfer_cmd = 7;

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
				+ ((address >> 8) & 0xF));
		    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

		    //Split word into 2 bytes and add them to buffer
		    for(i=0; i < 2; i++) {
		        p->spi_tx_buf[i + 2] = (unsigned char)((txd >> (i * 8)) & 0xFF);
		    }

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end
			break;
		default:
			break;
	}

    return error;
}

char CHIP_WriteByteArray_11(int num, unsigned short address, unsigned char *txd, unsigned short nBytes)
{
    char error = 0;
    unsigned short i;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = nBytes + 2;
			p->chip_transfer_rx_length = 0;
			p->chip_transfer_cmd = 11;

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
				+ ((address >> 8) & 0xF));
			p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

			for(i=2; i < p->chip_transfer_tx_length; i++) {
		        p->spi_tx_buf[i] = txd[i - 2];
		    }

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
			error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
		  	}
		  	//else
		  	p->chip_transfer_phase = PH30; //end
		  	break;
		default:
		  	break;
	}

    return error;
}

char CHIP_ReadByteArrayWithCRC_11(int num, unsigned short address, unsigned short nBytes, bool fromRam, bool *crcIsCorrect)
{
    char error = 0;
    unsigned short crcFromSpiSlave = 0, crcAtController = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			//first two bytes for sending command & address, third for size, last two bytes for CRC
			p->chip_transfer_tx_length = 3;
			p->chip_transfer_rx_length = nBytes + 2;
			p->chip_transfer_cmd = 100;

			memset((char *)&p->spi_tx_buf[0], 0, FPGA_SPI_BUFFER_LENGTH);

		    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ_CRC << 4)
				+ ((address >> 8) & 0xF));
		    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    		if(fromRam) {
        		p->spi_tx_buf[2] = nBytes >> 2;
		    } else {
    		    p->spi_tx_buf[2] = nBytes;
		    }

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

		    //Get CRC from controller
		    crcFromSpiSlave = (unsigned short)(p->spi_rx_buf[nBytes + 2] << 8)
				+ (unsigned short)(p->spi_rx_buf[nBytes + 3]);

		    //Use the receive buffer to calculate CRC
		    //First three bytes need to be command
		    p->spi_rx_buf[0] = p->spi_tx_buf[0];
		    p->spi_rx_buf[1] = p->spi_tx_buf[1];
		    p->spi_rx_buf[2] = p->spi_tx_buf[2];
		    crcAtController = CHIP_CalculateCRC16(p->spi_rx_buf, nBytes + 3);

		    if(crcFromSpiSlave == crcAtController) *crcIsCorrect = true;
		    else {
				*crcIsCorrect = false;
				printk(KERN_WARNING "rt_can_fd: CHIP_ReadByteArrayWithCRC_11 error2 %d, %x, %x\n",
					num, crcFromSpiSlave, crcAtController);
			}
			break;
		default:
			break;
	}

    return error;
}

char CHIP_ReadWordArray_11(int num, unsigned short address, unsigned int *rxd, unsigned short nWords)
{
    char error = 0;
    unsigned short i, j, n;
    T_CHIP_reg w;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			p->chip_transfer_tx_length = 2;
			p->chip_transfer_rx_length = nWords * 4;
			p->chip_transfer_cmd = 13;

			memset((char *)&p->spi_tx_buf[0], 0, FPGA_SPI_BUFFER_LENGTH);

		    p->spi_tx_buf[0] = (_CHIP_INS_READ << 4) + ((address >> 8) & 0xF);
		    p->spi_tx_buf[1] = address & 0xFF;

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
    		error = SPI_Transfer_11(num, p, 0);
			if(error < 0) { //fail
				p->chip_transfer_phase = PH20;
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

		    //Convert Byte array to Word array
		    n = 2;
		    for(i=0; i < nWords; i++) {
		        w.word = 0;
		        for(j=0; j < 4; j++, n++) {
        		    w.byte[j] = p->spi_rx_buf[n];
        		}
		        rxd[i] = w.word;
    		}
			break;
		default:
			break;
	}

    return error;
}

//Section : Configuration
char CHIP_Configure_11(int num, unsigned char log_flag)
{
    char error;
    T_CHIP_ctl ciCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

 	ciCon.word = canControlResetValues[_CHIP_REG_CiCON / 4];

	ciCon.bF.DNetFilterCount = p->config.DNetFilterCount;
	ciCon.bF.IsoCrcEnable = p->config.IsoCrcEnable;
    ciCon.bF.ProtocolExceptionEventDisable
		= p->config.ProtocolExpectionEventDisable;
    ciCon.bF.WakeUpFilterEnable = p->config.WakeUpFilterEnable;
    ciCon.bF.WakeUpFilterTime = p->config.WakeUpFilterTime;
    ciCon.bF.BitRateSwitchDisable = p->config.BitRateSwitchDisable;
    ciCon.bF.RestrictReTxAttempts = p->config.RestrictReTxAttempts;
    ciCon.bF.EsiInGatewayMode = p->config.EsiInGatewayMode;
    ciCon.bF.SystemErrorToListenOnly = p->config.SystemErrorToListenOnly;
    ciCon.bF.StoreInTEF = p->config.StoreInTEF;
    ciCon.bF.TXQEnable = p->config.TXQEnable;
    ciCon.bF.TxBandWidthSharing = p->config.TxBandWidthSharing;

    error = CHIP_WriteWord_11(num, _CHIP_REG_CiCON, ciCon.word);
    if(error < 0) { //fail
		printk(KERN_WARNING "rt_can_fd: CHIP_Configure_11 error %d, %d\n",
			num, error);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(log_flag == 1)
			printk(KERN_WARNING "rt_can_fd: %d CHIP_Configure_11\n", num);
	}

    return error;
}

char CHIP_ConfigureObjectReset(int num)
{
    T_CHIP_ctl ciCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    ciCon.word = canControlResetValues[_CHIP_REG_CiCON / 4];

    p->config.DNetFilterCount = ciCon.bF.DNetFilterCount;
    p->config.IsoCrcEnable = ciCon.bF.IsoCrcEnable;
    p->config.ProtocolExpectionEventDisable
		= ciCon.bF.ProtocolExceptionEventDisable;
    p->config.WakeUpFilterEnable = ciCon.bF.WakeUpFilterEnable;
    p->config.WakeUpFilterTime = ciCon.bF.WakeUpFilterTime;
    p->config.BitRateSwitchDisable = ciCon.bF.BitRateSwitchDisable;
    p->config.RestrictReTxAttempts = ciCon.bF.RestrictReTxAttempts;
    p->config.EsiInGatewayMode = ciCon.bF.EsiInGatewayMode;
    p->config.SystemErrorToListenOnly = ciCon.bF.SystemErrorToListenOnly;
    p->config.StoreInTEF = ciCon.bF.StoreInTEF;
    p->config.TXQEnable = ciCon.bF.TXQEnable;
    p->config.TxBandWidthSharing = ciCon.bF.TxBandWidthSharing;

    return 0;
}

//Section : Operating mode
char CHIP_OperationModeSelect_11(int num, T_CHIP_opMode opMode, unsigned char log_flag)
{
    char error = 0;
    unsigned char d = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH3:
		    error = CHIP_ReadByte_11(num, _CHIP_REG_CiCON + 3, &p->chip_tmp_data_uc1);
			if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_OperationModeSelect_11 error1 %d, %d\n", num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH4:
		    //Modify
		    d = p->chip_tmp_data_uc1;
		    d &= ~0x07;
			d |= opMode;

		    error = CHIP_WriteByte_11(num, _CHIP_REG_CiCON + 3, d);
			if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_OperationModeSelect_11 error2 %d, %d\n", num, error);
				break;
			} else if(error == 0) {
				break; //wait
			}

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_OperationModeSelect_11 value %d, %x, %x\n", num, _CHIP_REG_CiCON + 3, d);

			}

			p->chip_transfer_phase = PH0;
			p->chip_op_phase++;
			break;
		default:
			break;
	}

    return error;
}

//Section : CAN Transmit
char CHIP_TransmitChannelConfigure_11(int num, T_CHIP_fifoChannel channel, unsigned char log_flag)
{
    char error;
    unsigned short a;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Setup FIFO
    ciFifoCon.word = canFifoResetValues[0];
    ciFifoCon.txBF.TxEnable = 1;
    ciFifoCon.txBF.FifoSize = p->txConfig.FifoSize;
    ciFifoCon.txBF.PayLoadSize = p->txConfig.PayLoadSize;
    ciFifoCon.txBF.TxAttempts = p->txConfig.TxAttempts;
    ciFifoCon.txBF.TxPriority = p->txConfig.TxPriority;
    ciFifoCon.txBF.RTREnable = p->txConfig.RTREnable;

    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_WriteWord_11(num, a, ciFifoCon.word);
	if(error < 0) { //fail
		printk(KERN_WARNING "rt_can_fd: CHIP_TransmitChannelConfigure_11 error %d, %d\n", num, error);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(log_flag == 1) {
			printk(KERN_WARNING
				"rt_can_fd: %d TransmitChannelConfigure_11\n", num);
		}
	}

    return error;
}

char CHIP_TransmitChannelConfigureObjectReset(int num)
{
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    ciFifoCon.word = canFifoResetValues[0];

    p->txConfig.RTREnable = ciFifoCon.txBF.RTREnable;
    p->txConfig.TxPriority = ciFifoCon.txBF.TxPriority;
    p->txConfig.TxAttempts = ciFifoCon.txBF.TxAttempts;
    p->txConfig.FifoSize = ciFifoCon.txBF.FifoSize;
    p->txConfig.PayLoadSize = ciFifoCon.txBF.PayLoadSize;

    return 0;
}

char CHIP_MCU_TransmitMessage(int num)
{
    char error = 0;
    unsigned short n, i, j;
    unsigned int idx, txdNumBytes;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_transfer_phase) {
		case PH0:
			idx = p->tx_msg_read_idx + 1;
			if(idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) idx = 0;

			memset((char *)&p->spi_tx_buf[0], 0, FPGA_SPI_BUFFER_LENGTH);
			memcpy((char *)&p->spi_tx_buf[0], (char *)&p->txObj.byte[0], 8);

			txdNumBytes = CHIP_DlcToDataBytes(p->txObj.bF.ctrl.DLC);
			memcpy((char *)&p->spi_tx_buf[8], (char *)&p->tx_buf[idx].data[0],
				txdNumBytes);

		    //Make sure we write a multiple of 4 bytes to RAM
			n = 0;
		    if(txdNumBytes % 4) { //kjg_240508
				//Need to add bytes
				n = 4 - (txdNumBytes % 4);
				i = txdNumBytes + 8;

				for(j=0; j < n; j++) {
					p->spi_tx_buf[i + 8 + j] = 0;
				}
			}

			p->chip_transfer_tx_length = txdNumBytes + 8 + n;
			p->chip_transfer_rx_length = 0;
			p->chip_transfer_cmd = 112;

			p->chip_transfer_phase++;
			//skip break;
		case PH1:
		case PH2:
		case PH3:
		case PH4:
			error = SPI_Transfer_TX(num, p, 0);
			if(error < 0) {
				p->chip_transfer_phase = PH20; //fail
				break;
			} else if(error == 0) { //kjg_240526
				break; //wait
			}
			//else
			p->chip_transfer_phase = PH30; //end

			idx = p->tx_msg_read_idx + 1;
			if(idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) idx = 0;

			p->tx_msg_read_idx = idx;
			if(p->tx_msg_count > 0) p->tx_msg_count--;
			break;
		default:
			break;
	}

    return error;
}

//Section : CAN Receive
char CHIP_FilterObjectConfigure_11(int num, T_CHIP_filter filter, unsigned char log_flag)
{
    char error;
    unsigned short a;
    T_CHIP_filtObj fObj;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Setup
    fObj.word = 0;
    fObj.bF = p->fObj.bF;
    a = _CHIP_REG_CiFLTOBJ + (filter * _CHIP_FILTER_OFFSET);

    error = CHIP_WriteWord_11(num, a, fObj.word);
	if(error < 0) { //fail
		printk(KERN_WARNING
			"rt_can_fd: CHIP_FilterObjectConfigure_11 error ch%d %d %d\n",
			num, filter, error);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(log_flag == 1) {
			printk(KERN_WARNING "rt_can_fd: ch%d %d FilterObjectConfigure_11 a:%x %x %x %d %d\n",
				num, filter, a, fObj.bF.SID, fObj.bF.EID, fObj.bF.SID11, fObj.bF.EXIDE);
		}
	}

    return error;
}

char CHIP_FilterMaskConfigure_11(int num, T_CHIP_filter filter, unsigned char log_flag)
{
    char error;
    unsigned short a;
    T_CHIP_maskObj mObj;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Setup
    mObj.word = 0;
    mObj.bF = p->mObj.bF;
    a = _CHIP_REG_CiMASK + (filter * _CHIP_FILTER_OFFSET);

    error = CHIP_WriteWord_11(num, a, mObj.word);
	if(error < 0) { //fail
		printk(KERN_WARNING
			"rt_can_fd: CHIP_FilterMaskConfigure_11 error ch%d %d %d\n",
			num, filter, error);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(log_flag == 1) {
			printk(KERN_WARNING "rt_can_fd: ch%d %d FilterMaskConfigure_11 a:%x %x %x %d %d\n",
				num, filter, a, mObj.bF.MSID, mObj.bF.MEID, mObj.bF.MSID11, mObj.bF.MIDE);
		}
	}

    return error;
}

char CHIP_FilterToFifoLink_11(int num, T_CHIP_filter filter, T_CHIP_fifoChannel channel, bool enable, unsigned char log_flag)
{
    char error;
    unsigned short a;
    T_CHIP_filtCtl fCtrl;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    if(enable) fCtrl.bF.Enable = 1;
    else fCtrl.bF.Enable = 0;

    //Link
    fCtrl.bF.BufferPointer = channel;
    a = _CHIP_REG_CiFLTCON + filter;

    error = CHIP_WriteByte_11(num, a, fCtrl.byte);
    if(error < 0) { //fail
		printk(KERN_WARNING
			"rt_can_fd: CHIP_FilterToFifoLink_11 error ch%d %d %d\n",
			num, filter, error);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(log_flag == 1) {
			printk(KERN_WARNING "rt_can_fd: ch%d %d FilterToFifoLink_11 %d %d, a:%x %d %d\n",
				num, filter, p->set_phase, p->chip_op_phase, a, fCtrl.bF.BufferPointer, fCtrl.bF.Enable);
		}
	}

    return error;
}

char CHIP_ReceiveChannelConfigure_11(int num, T_CHIP_fifoChannel channel, unsigned char log_flag)
{
    char error;
    unsigned short a;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelConfigure_11 error1 %d\n", num);
		return -100;
	}

    //Setup FIFO
    ciFifoCon.word = canFifoResetValues[0];

    ciFifoCon.rxBF.TxEnable = 0;
    ciFifoCon.rxBF.FifoSize = p->rxConfig.FifoSize;
    ciFifoCon.rxBF.PayLoadSize = p->rxConfig.PayLoadSize;
    ciFifoCon.rxBF.RxTimeStampEnable = p->rxConfig.RxTimeStampEnable;

    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_WriteWord_11(num, a, ciFifoCon.word);
	if(error < 0) { //fail
		printk(KERN_WARNING "rt_can_fd: CHIP_ReceiveChannelConfigure_11 error2 %d, %d\n", num, error);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(log_flag == 1) {
			printk(KERN_WARNING
				"rt_can_fd: %d ReceiveChannelConfigure_11\n", num);
		}
	}

    return error;
}

char CHIP_ReceiveChannelConfigureObjectReset(int num)
{
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    ciFifoCon.word = canFifoResetValues[0];

    p->rxConfig.FifoSize = ciFifoCon.rxBF.FifoSize;
    p->rxConfig.PayLoadSize = ciFifoCon.rxBF.PayLoadSize;
    p->rxConfig.RxTimeStampEnable = ciFifoCon.rxBF.RxTimeStampEnable;

    return 0;
}

//Section : Module Events
char CHIP_ModuleEventEnable_11(int num, T_CHIP_moduleEvent flags, unsigned char log_flag)
{
    char error = 0;
    unsigned short a;
    T_CHIP_intCfg intEnables;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
		    //Read Interrupt Enables
		    a = _CHIP_REG_CiINTENABLE;
		    error = CHIP_ReadHalfWord_11(num, a, &p->chip_tmp_data_us1);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ModuleEventEnable_11 error1 %d, %d\n", num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				//printk(KERN_WARNING
				//	"rt_can_fd: CHIP_ModuleEventEnable_11 CiINTENABLE1 %d, %x\n", num, p->chip_tmp_data_us1);

				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH1:
		    a = _CHIP_REG_CiINTENABLE;
		    intEnables.word = p->chip_tmp_data_us1;

		    //Modify
		    intEnables.word |= (flags & CHIP_ALL_EVENTS);

		    error = CHIP_WriteHalfWord_11(num, a, intEnables.word);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ModuleEventEnable_11 error2 %d, %d\n", num, error);
				break;
			} else if(error == 0) {
				break; //wait
			}

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING "rt_can_fd: CHIP_ModuleEventEnable_11 CiINTENABLE2 %d, %x, %x\n", num, p->chip_tmp_data_us1, intEnables.word);
			}

			p->chip_op_phase++;
			break;
		default:
			break;
	}

    return error;
}

//Section : Transmit FIFO Events
char CHIP_TransmitChannelEventGet_11(int num, T_CHIP_fifoChannel channel, T_CHIP_txFifoEvent *flags, unsigned char log_flag)
{
    char error = 0;
    unsigned short a;
    T_CHIP_fifoStat ciFifoSta;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH1:
		    //Read Interrupt flags
		    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
		    error = CHIP_ReadByte_11(num, a, &p->chip_tmp_data_uc1);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_TransmitChannelEventGet_11 error %d, %d\n",
					num, error);
				return -1;
			} else if(error == 0) {
				break; //wait
			}

		    //Update data
		    ciFifoSta.word = 0;
			ciFifoSta.byte[0] = p->chip_tmp_data_uc1;
		    *flags = (T_CHIP_txFifoEvent)(ciFifoSta.byte[0]
				& CHIP_TX_FIFO_ALL_EVENTS);

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: %d TransmitChannelEventGet_11\n", num);
			}

			p->chip_op_phase++;
			break;
		default:
			break;
	}

    return error;
}

char CHIP_TransmitChannelEventEnable_11(int num, T_CHIP_fifoChannel channel, T_CHIP_txFifoEvent flags, unsigned char log_flag)
{
    char error = 0;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH3:
		    //Read Interrupt Enables
		    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
		    error = CHIP_ReadByte_11(num, a, &p->chip_tmp_data_uc1);
		    if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: CHIP_TransmitChannelEventEnable_11 error1 %d, %d\n", num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH4:
		    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    		//Modify
			ciFifoCon.word = 0;
			ciFifoCon.byte[0] = p->chip_tmp_data_uc1;
		    ciFifoCon.byte[0] |= (flags & CHIP_TX_FIFO_ALL_EVENTS);

		    error = CHIP_WriteByte_11(num, a, ciFifoCon.byte[0]);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_TransmitChannelEventEnable_11 error2 %d, %d\n",
					num, error);
				return -2;
			} else if(error == 0) {
				break; //wait
			}

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING "rt_can_fd: CHIP_TransmitChannelEventEnable_11 value %d, %x, %x\n",
					num, a, ciFifoCon.byte[0]);
			}

			p->chip_op_phase++;
			break;
		default:
			break;
	}

    return error;
}

char CHIP_TransmitChannelEventDisable_11(int num, T_CHIP_fifoChannel channel, T_CHIP_txFifoEvent flags, unsigned char log_flag)
{
    char error = 0;
    unsigned short a;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH3:
		    //Read Interrupt Enables
		    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
		    error = CHIP_ReadByte_11(num, a, &p->chip_tmp_data_uc1);
    		if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: CHIP_TransmitChannelEventDisable_11 error1 %d, %d\n", num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH4:
		    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
		    ciFifoCon.word = 0;
			ciFifoCon.byte[0] = p->chip_tmp_data_uc1;

		    //Modify
    		ciFifoCon.byte[0] &= ~(flags & CHIP_TX_FIFO_ALL_EVENTS);

		    error = CHIP_WriteByte_11(num, a, ciFifoCon.byte[0]);
		    if(error < 0) { //fail
				printk(KERN_WARNING "rt_can_fd: CHIP_TransmitChannelEventDisable_11 error2 %d, %d\n", num, error);
				break;
			} else if(error == 0) {
				break; //wait
			}

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: %d TransmitChannelEventDisable_11\n", num);
			}

			p->chip_op_phase++;
			break;
		default:
			break;
	}

    return error;
}

//Section : Receive FIFO Events
char CHIP_ReceiveChannelEventGet_11(int num, T_CHIP_fifoChannel channel, T_CHIP_rxFifoEvent *flags, unsigned char log_flag)
{
    char error = 0;
    unsigned short a;
    T_CHIP_fifoStat ciFifoSta;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventGet_11 error1) %d, %d\n",
			num, channel);
		return -100;
	}

    //Read Interrupt flags
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    error = CHIP_ReadByte_11(num, a, &p->chip_tmp_data_uc1);
    if(error < 0) { //fail
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventGet_11 error2 %d, %d\n",
			num, error);
		return -1;
	} else if(error == 0) return 0; //wait

    ciFifoSta.word = 0;
    ciFifoSta.byte[0] = p->chip_tmp_data_uc1;

	//printk(KERN_WARNING
	//	"rt_can_fd: CHIP_ReceiveChannelEventGet_11 value %d, %x\n",
	//	num, p->chip_tmp_data_uc1);
	
    //Update data
    *flags = (T_CHIP_rxFifoEvent)(ciFifoSta.byte[0] & CHIP_RX_FIFO_ALL_EVENTS);

	if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
	if(log_flag == 1) {
		printk(KERN_WARNING
			"rt_can_fd: %d CHIP_ReceiveChannelEventGet_11\n", num);
	}

    return error;
}

char CHIP_ReceiveChannelEventEnable_11(int num, T_CHIP_fifoChannel channel, T_CHIP_rxFifoEvent flags, unsigned char log_flag)
{
    char error = 0;
    unsigned short a;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH6:
		    if(channel == CAN_TXQUEUE_CH0) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ReceiveChannelEventEnable_11 error1 %d\n", num);
				return -100;
			}

			p->chip_op_phase++;
			//skip break;
		case PH7:
		    //Read Interrupt Enables
		    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
		    error = CHIP_ReadByte_11(num, a, &p->chip_tmp_data_uc1);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ReceiveChannelEventEnable_11 error2 %d, %d\n",
					num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH8:
		    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
		    ciFifoCon.word = 0;
			ciFifoCon.byte[0] = p->chip_tmp_data_uc1;

		    //Modify
    		ciFifoCon.byte[0] |= (flags & CHIP_RX_FIFO_ALL_EVENTS);

		    error = CHIP_WriteByte_11(num, a, ciFifoCon.byte[0]);
 		   	if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ReceiveChannelEventEnable_11 error3 %d, %d\n",
					num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				//printk(KERN_WARNING
				//	"rt_can_fd: CHIP_ReceiveChannelEventEnable_11 value1 %d, %x, %x\n",
				//	num, a, ciFifoCon.byte[0]);

				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH9:
		    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
		    ciFifoCon.word = 0;
		    error = CHIP_ReadByte_11(num, a, &ciFifoCon.byte[0]);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ReceiveChannelEventEnable_11 error2 %d, %d\n",
					num, error);
				break;
			} else if(error == 0) break; //wait

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING "rt_can_fd: CHIP_ReceiveChannelEventEnable_11 value2 %d, %x, %x\n",
					num, a, ciFifoCon.byte[0]);
			}

			p->chip_transfer_phase = PH0;
			p->chip_op_phase++;
			break;
		default:
			break;
	}

    return error;
}

//Section : Error Handling
char CHIP_ErrorCountStateGet_11(int num, unsigned char *tec, unsigned char *rec, T_CHIP_errorState *flags)
{
    char error;
    unsigned short a;
    T_CHIP_trec ciTrec;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Read Error
    a = _CHIP_REG_CiTREC;
    error = CHIP_ReadWord_11(num, a, &p->chip_tmp_data_ui1);
    if(error < 0) { //fail
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ErrorCountStateGet_11 error %d, %d\n", num, error);
		return -1;
	} else if(error == 0) return 0; //wait

    ciTrec.word = p->chip_tmp_data_ui1;

    //Update data
    *tec = ciTrec.byte[1];
    *rec = ciTrec.byte[0];
    *flags = (T_CHIP_errorState)(ciTrec.byte[2] & CHIP_ERROR_ALL);

    return error;
}

//Section : ECC
char CHIP_EccEnable_11(int num, unsigned char log_flag)
{
    char error = 0;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
			error = CHIP_ReadByte_11(num, _CHIP_REG_ECCCON,
				&p->chip_tmp_data_uc1);
			if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_EccEnable_11 error1 %d, %d\n", num, error);
				return -1;
			} else if(error == 0) break; //wait

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_EccEnable_11 ECCCON1 %d, %x\n",
					num, p->chip_tmp_data_uc1);
			}
	
		  	//Modify
		    p->chip_tmp_data_uc1 |= 0x01;

			p->chip_transfer_phase = PH0;
			p->chip_op_phase++;
			error = 0;
			break;
		case PH1:
		    error = CHIP_WriteByte_11(num, _CHIP_REG_ECCCON,
				p->chip_tmp_data_uc1);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_EccEnable_11 error2 %d, %d\n", num, error);
				return -2;
			} else if(error == 0) break; //wait

			if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
			if(log_flag == 1) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_EccEnable_11 ECCCON2 %d, %x\n",
					num, p->chip_tmp_data_uc1);
			}

			p->chip_op_phase++;
			break;
		default:
			break;
	}

    return error;
}

char CHIP_RamInit_11(int num, unsigned char log_flag)
{
    char error = 0;
    //kjg_240213 unsigned char txd[CHIP_SPI_BUFFER_LENGTH];
    unsigned char txd[32];
    unsigned short a;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

   	//Prepare data
	//kjg_240213 memset((char *)&txd[0], 0xFF, CHIP_SPI_BUFFER_LENGTH);
	memset((char *)&txd[0], 0xFF, 32);

	//set_phase 10~73
	a = _CHIP_RAMADDR_START + (32 * (p->set_phase - 10));

    error = CHIP_WriteByteArray_11(num, a, txd, 32);
    if(error < 0) { //fail
		printk(KERN_WARNING
			"rt_can_fd: CHIP_RamInit_11 error %d, %d, %d\n",
			num, error, p->set_phase);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(p->set_phase == PH73 && log_flag == 1)
			printk(KERN_WARNING "rt_can_fd: %d RamInit_11\n", num);
	}

    return error;
}

//Section : Bit Time
char CHIP_BitTimeConfigureNominal40MHz_11(int num, unsigned char log_flag)
{
	unsigned char tmp1, kjg_tmp;
    char error = 0;
	unsigned int _ch = 0, _can_type = 0; //ksh_241112
    T_CHIP_nbtCfg ciNbtcfg;
	struct rt_can_fd_struct *p;
	int manual_can_bitflag = 0;

	//ksh_241112_s
	if(myData->mData.config.division_CAN == 1) { //Only Master
		_ch = num;	//ch
		_can_type = 0; //Master
	} else { //Master and Slave
		_ch = num/2;
		_can_type = num%2;
	}

	manual_can_bitflag = (int) myData->canTransmitSetData.commonData[_ch][_can_type].manual_can_bitconfig;
	//ksh_241112_e

	p = &(rt_can_fd_table[num]);

    ciNbtcfg.word = canControlResetValues[_CHIP_REG_CiNBTCFG / 4];
	switch(manual_can_bitflag) {
		case P0:
		    switch(p->selectedBitTime) { //Arbitration Bit rate
		        case CHIP_125K_500K:
		            ciNbtcfg.bF.BRP = 0;
		            ciNbtcfg.bF.TSEG1 = 254;
		            ciNbtcfg.bF.TSEG2 = 63;
		            ciNbtcfg.bF.SJW = 63;
		            break;
		        case CHIP_250K_500K:
		        case CHIP_250K_833K:
		        case CHIP_250K_1M:
		        case CHIP_250K_1M5:
		        case CHIP_250K_2M:
		        case CHIP_250K_3M:
		        case CHIP_250K_4M:
		            ciNbtcfg.bF.BRP = 0;
		            ciNbtcfg.bF.TSEG1 = 126;
		            ciNbtcfg.bF.TSEG2 = 31;
		            ciNbtcfg.bF.SJW = 31;
		            break;
		        case CHIP_500K_1M:
		        //case CHIP_500K_2M:	//sec_221101
		        case CHIP_500K_3M:
		        case CHIP_500K_4M:
		        case CHIP_500K_5M:
		        case CHIP_500K_6M7:
		        case CHIP_500K_8M:
		        case CHIP_500K_10M:
		            ciNbtcfg.bF.BRP = 0;
		            ciNbtcfg.bF.TSEG1 = 62;
		            ciNbtcfg.bF.TSEG2 = 15;
		            ciNbtcfg.bF.SJW = 15;
		            break;
		        case CHIP_500K_2M:	//sec_221101
		            ciNbtcfg.bF.BRP = 0;
		            ciNbtcfg.bF.TSEG1 = 54;
		            ciNbtcfg.bF.TSEG2 = 23;
		            ciNbtcfg.bF.SJW = 23;
		            break;
				case CHIP_500K_3M2: //kjg_220714
					tmp1 = 0x10;
					if(num <= 3) {
						if((terminal_resistor[0] & (tmp1 << num)) == 0) kjg_tmp = 0;
						else kjg_tmp = 1;
					} else {
						if((terminal_resistor[1] & (tmp1 << (num - 4))) == 0)
							kjg_tmp = 0;
						else kjg_tmp = 1;
					}

					if(kjg_tmp == 0) {
		       	    	ciNbtcfg.bF.BRP = 0;
		        	    ciNbtcfg.bF.TSEG1 = 62;
		            	ciNbtcfg.bF.TSEG2 = 15;
			            ciNbtcfg.bF.SJW = 15;
					} else {
		       	    	ciNbtcfg.bF.BRP = 0;
		        	    ciNbtcfg.bF.TSEG1 = 59;
		            	ciNbtcfg.bF.TSEG2 = 14;
			            ciNbtcfg.bF.SJW = 14;
					}
					break;
		        case CHIP_1000K_4M:
		        case CHIP_1000K_8M:
		            ciNbtcfg.bF.BRP = 0;
		            ciNbtcfg.bF.TSEG1 = 30;
		            ciNbtcfg.bF.TSEG2 = 7;
		            ciNbtcfg.bF.SJW = 7;
		            break;
		        default:
					printk(KERN_WARNING "rt_can_fd: CHIP_BitTimeConfigureNominal40MHz_11 error1 %d, %d\n", num, p->selectedBitTime);
		            return -1;
		            break;
		    }
			break;
	case P1: //Manual CAN Configuration ON //ksh_241112
		
		//Data BR
		ciNbtcfg.bF.BRP = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].nominal_brp;
        ciNbtcfg.bF.TSEG1 = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].nominal_tseg1;
        ciNbtcfg.bF.TSEG2 = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].nominal_tseg2;
        ciNbtcfg.bF.SJW = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].nominal_sjw;
		break;
	}

    //Write Bit time registers
    error = CHIP_WriteWord_11(num, _CHIP_REG_CiNBTCFG, ciNbtcfg.word);
	if(error < 0) { //fail
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureNominal40MHz_11 error2 %d, %d\n",
			num, error);
	} else if(error == 0) {} //wait
	else { //end
		if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
		if(log_flag == 1) {
			printk(KERN_WARNING "rt_can_fd: CHIP_BitTimeConfigureNominal40MHz_11 %d, %d, %x, %d\n", num, p->selectedBitTime, ciNbtcfg.word, error);
		}
	}

    return error;
}

char CHIP_BitTimeConfigureData40MHz_11(int num, T_CHIP_sspMode sspMode, unsigned char log_flag)
{
	unsigned char tmp1, kjg_tmp;
    char error = 0;
    unsigned int tdcValue = 0;
	unsigned int _ch = 0, _can_type = 0; //ksh_241112
    T_CHIP_dbtCfg ciDbtcfg;
    T_CHIP_tdcCfg ciTdc;
	struct rt_can_fd_struct *p;
	int manual_can_bitflag = 0;

	//ksh_241112_s
	if(myData->mData.config.division_CAN == 1) { //Only Master
		_ch = num;	//ch
		_can_type = 0; //Master
	} else { //Master and Slave
		_ch = num/2;
		_can_type = num%2;
	}
	
	if(myData->canTransmitSetData.commonData[_ch][_can_type].can_fd_flag == 0) { //CAN FD installed, disfunctional
        manual_can_bitflag = 0;
    } else { //Fully filled on manual Can bit Configuration
        manual_can_bitflag = (int) myData->canTransmitSetData.commonData[_ch][_can_type].manual_can_bitconfig;
    }
	//ksh_241112_e

	p = &(rt_can_fd_table[num]);
    
    //sspMode;
    ciDbtcfg.word = canControlResetValues[_CHIP_REG_CiDBTCFG / 4];
    ciTdc.word = 0;

    //Configure Bit time and sample point
    //kjg_220701 ciTdc.bF.TDCMode = CHIP_SSP_MODE_AUTO;
    ciTdc.bF.TDCMode = sspMode;
	switch(manual_can_bitflag){
		case P0:
		    switch(p->selectedBitTime) { //Data Bit rate and SSP
		        case CHIP_125K_500K:
		        case CHIP_250K_500K:
					//Data BR
		            ciDbtcfg.bF.BRP = 1;
		            ciDbtcfg.bF.TSEG1 = 30;
		            ciDbtcfg.bF.TSEG2 = 7;
		            ciDbtcfg.bF.SJW = 7;
		            //SSP
		            ciTdc.bF.TDCOffset = 31;
		            ciTdc.bF.TDCValue = tdcValue;
		            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
		            break;
		        case CHIP_250K_833K:
					//Data BR
		            ciDbtcfg.bF.BRP = 1;
		            ciDbtcfg.bF.TSEG1 = 17;
		            ciDbtcfg.bF.TSEG2 = 4;
		            ciDbtcfg.bF.SJW = 4;
		            //SSP
		            ciTdc.bF.TDCOffset = 18;
		            ciTdc.bF.TDCValue = tdcValue;
		            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
		            break;
		        case CHIP_250K_1M:
		        case CHIP_500K_1M:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 30;
		            ciDbtcfg.bF.TSEG2 = 7;
		            ciDbtcfg.bF.SJW = 7;
		            //SSP
		            ciTdc.bF.TDCOffset = 31;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
		        case CHIP_250K_1M5:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 18;
		            ciDbtcfg.bF.TSEG2 = 5;
		            ciDbtcfg.bF.SJW = 5;
		            //SSP
		            ciTdc.bF.TDCOffset = 19;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
		        case CHIP_250K_2M:
		        //case CHIP_500K_2M:	//sec_221101r
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 14;
		            ciDbtcfg.bF.TSEG2 = 3;
		            ciDbtcfg.bF.SJW = 3;
		            //SSP
		            ciTdc.bF.TDCOffset = 15;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
		        case CHIP_500K_2M:		//sec_221101
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 13;
		            ciDbtcfg.bF.TSEG2 = 4;
		            ciDbtcfg.bF.SJW = 4;
		            //SSP
		            ciTdc.bF.TDCOffset = 14;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
		        case CHIP_250K_3M:
		        case CHIP_500K_3M:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 8;
		            ciDbtcfg.bF.TSEG2 = 2;
		            ciDbtcfg.bF.SJW = 2;
		            //SSP
		            ciTdc.bF.TDCOffset = 9;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
				case CHIP_500K_3M2: //kjg_220714
					tmp1 = 0x10;
					if(num <= 3) {
						if((terminal_resistor[0] & (tmp1 << num)) == 0) kjg_tmp = 0;
						else kjg_tmp = 1;
					} else {
						if((terminal_resistor[1] & (tmp1 << (num - 4))) == 0)
							kjg_tmp = 0;
						else kjg_tmp = 1;
					}

					if(kjg_tmp == 0) {
						//Data BR
		    	        ciDbtcfg.bF.BRP = 0;
		        	    ciDbtcfg.bF.TSEG1 = 8;
		            	ciDbtcfg.bF.TSEG2 = 2;
			            ciDbtcfg.bF.SJW = 2;
		    	        //SSP
		        	    ciTdc.bF.TDCOffset = 9;
		            	ciTdc.bF.TDCValue = tdcValue;
					} else {
						//Data BR
		    	        ciDbtcfg.bF.BRP = 0;
		        	    ciDbtcfg.bF.TSEG1 = 8;
		            	ciDbtcfg.bF.TSEG2 = 1;
			            ciDbtcfg.bF.SJW = 1;
		    	        //SSP
		        	    ciTdc.bF.TDCOffset = 9;
		            	ciTdc.bF.TDCValue = tdcValue;
					}
					break;
		        case CHIP_250K_4M:
		        case CHIP_500K_4M:
		        case CHIP_1000K_4M:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 6;
		            ciDbtcfg.bF.TSEG2 = 1;
		            ciDbtcfg.bF.SJW = 1;
		            //SSP
		            ciTdc.bF.TDCOffset = 7;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
		        case CHIP_500K_5M:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 4;
		            ciDbtcfg.bF.TSEG2 = 1;
		            ciDbtcfg.bF.SJW = 1;
		            //SSP
		            ciTdc.bF.TDCOffset = 5;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
		        case CHIP_500K_6M7:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 3;
		            ciDbtcfg.bF.TSEG2 = 0;
		            ciDbtcfg.bF.SJW = 0;
		            //SSP
		            ciTdc.bF.TDCOffset = 4;
		            ciTdc.bF.TDCValue = tdcValue;
		            break;
		        case CHIP_500K_8M:
		        case CHIP_1000K_8M:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 2;
		            ciDbtcfg.bF.TSEG2 = 0;
		            ciDbtcfg.bF.SJW = 0;
		            //SSP
		            ciTdc.bF.TDCOffset = 3;
		            ciTdc.bF.TDCValue = 1;
		            break;
		        case CHIP_500K_10M:
					//Data BR
		            ciDbtcfg.bF.BRP = 0;
		            ciDbtcfg.bF.TSEG1 = 1;
		            ciDbtcfg.bF.TSEG2 = 0;
		            ciDbtcfg.bF.SJW = 0;
		            //SSP
		            ciTdc.bF.TDCOffset = 2;
		            ciTdc.bF.TDCValue = 0;
		            break;
		        default:
					printk(KERN_WARNING
						"rt_can_fd: CHIP_BitTimeConfigureData40MHz_11 error1 %d, %d\n",
						num, p->selectedBitTime);
		            return -1;
		            break;
			}
			break;
		case P1: //Manual CAN BitConfiguration ON //ksh_241112			
			
			//Data BR
	        ciDbtcfg.bF.BRP = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].data_brp;
	        ciDbtcfg.bF.TSEG1 = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].data_tseg1;
	        ciDbtcfg.bF.TSEG2 = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].data_tseg2;
	        ciDbtcfg.bF.SJW = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].data_sjw;
	        //SSP
	        ciTdc.bF.TDCMode = P2; //fixed
	        ciTdc.bF.TDCOffset = (unsigned int)myData->canTransmitSetData.commonData[_ch][_can_type].data_tseg1 + 1;
			ciTdc.bF.TDCValue = tdcValue; //fixed
			
			break;
	}
	switch(p->chip_op_phase) {
		case PH2:
		    //Write Bit time registers
		    error = CHIP_WriteWord_11(num, _CHIP_REG_CiDBTCFG, ciDbtcfg.word);
    		if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_BitTimeConfigureData40MHz_11 error2 %d, %d, %d\n",
					num, p->selectedBitTime, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH3:
		    //Write Transmitter Delay Compensation
#ifdef REV_A
	    	ciTdc.bF.TDCOffset = 0;
    		ciTdc.bF.TDCValue = 0;
#endif

		    error = CHIP_WriteWord_11(num, _CHIP_REG_CiTDC, ciTdc.word);
    		if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_BitTimeConfigureData40MHz_11 error3 %d, %d, %d\n",
					num, p->selectedBitTime, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING "rt_can_fd: CHIP_BitTimeConfigureData40MHz_11 %d, %d, %x, %x\n",
						num, p->selectedBitTime, ciDbtcfg.word, ciTdc.word);
				}
			}
			break;
		default:
			break;
	}

    return error;
}

//Section : GPIO
char CHIP_GpioModeConfigure_11(int num, T_CHIP_pinMode gpio0, T_CHIP_pinMode gpio1, unsigned char log_flag)
{
    char error = 0;
    unsigned short a;
    T_CHIP_ioCtl iocon;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(p->chip_op_phase) {
		case PH0:
 			a = _CHIP_REG_IOCON + 3;
    		error = CHIP_ReadByte_11(num, a, &p->chip_tmp_data_uc1);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_GpioModeConfigure_11 error1 %d, %d\n", num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				p->chip_transfer_phase = PH0;
				p->chip_op_phase++;
				error = 0;
			}
			break;
		case PH1:
 			a = _CHIP_REG_IOCON + 3;
		    iocon.word = 0;
			iocon.byte[3] = p->chip_tmp_data_uc1;

		    //Modify
		    iocon.bF.PinMode0 = gpio0;
		    iocon.bF.PinMode1 = gpio1;

		    error = CHIP_WriteByte_11(num, a, iocon.byte[3]);
		    if(error < 0) { //fail
				printk(KERN_WARNING
					"rt_can_fd: CHIP_GpioModeConfigure_11 error2 %d, %d\n", num, error);
			} else if(error == 0) {
				break; //wait
			} else { //end
				if(log_ch != 0 && num != (log_ch - 1)) log_flag = 0;
				if(log_flag == 1) {
					printk(KERN_WARNING
						"rt_can_fd: %d GpioModeConfigure_11\n", num);
				}
				p->chip_op_phase++;
			}
			break;
		default:
			break;
	}

    return error;
}

//Section : Miscellaneous
unsigned int CHIP_DlcToDataBytes(T_CHIP_dlc dlc)
{
    unsigned int dataBytesInObject;

    if(dlc < CHIP_DLC_12) {
        dataBytesInObject = dlc;
    } else {
        switch(dlc) {
            case CHIP_DLC_12: dataBytesInObject = 12; break;
            case CHIP_DLC_16: dataBytesInObject = 16; break;
            case CHIP_DLC_20: dataBytesInObject = 20; break;
            case CHIP_DLC_24: dataBytesInObject = 24; break;
            case CHIP_DLC_32: dataBytesInObject = 32; break;
            case CHIP_DLC_48: dataBytesInObject = 48; break;
            case CHIP_DLC_64: dataBytesInObject = 64; break;
            default: dataBytesInObject = 0; break;
        }
    }

    return dataBytesInObject;
}

unsigned short CHIP_CalculateCRC16(unsigned char *data, unsigned short size)
{
    unsigned char index;
    unsigned short init = CRCBASE;

    while(size-- != 0) {
        index = ((unsigned char *)&init)[CRCUPPER] ^ *data++;
        init = (init << 8) ^ crc16Table[index];
    }

    return init;
}

T_CHIP_dlc CHIP_DataBytesToDlc(unsigned char n)
{
    T_CHIP_dlc dlc;

    if(n <= 8) {
		switch(n) {
			case 0: dlc = CHIP_DLC_0; break;
			case 1: dlc = CHIP_DLC_1; break;
			case 2: dlc = CHIP_DLC_2; break;
			case 3: dlc = CHIP_DLC_3; break;
			case 4: dlc = CHIP_DLC_4; break;
			case 5: dlc = CHIP_DLC_5; break;
			case 6: dlc = CHIP_DLC_6; break;
			case 7: dlc = CHIP_DLC_7; break;
			case 8: dlc = CHIP_DLC_8; break;
			default: dlc = CHIP_DLC_0; break;
		}
    } else if(n <= 12) {
        dlc = CHIP_DLC_12;
    } else if(n <= 16) {
        dlc = CHIP_DLC_16;
    } else if(n <= 20) {
        dlc = CHIP_DLC_20;
    } else if(n <= 24) {
        dlc = CHIP_DLC_24;
    } else if(n <= 32) {
        dlc = CHIP_DLC_32;
    } else if(n <= 48) {
        dlc = CHIP_DLC_48;
    } else if(n <= 64) {
        dlc = CHIP_DLC_64;
    } else {
		dlc = CHIP_DLC_0;
	}

    return dlc;
}

int rt_can_exec_cmd_reset(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd, kjg_val[8], kjg_val2[4];
	int addr, i;

	tx_count = 3 + 2;
	rx_count = 1;
	tx_cmd = 0x00; //chip command
	addr = RT_CAN_FD_BASE_ADDR + 0x40 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	outb(0x00, addr+4); //reset
	outb(0x00, addr+4);

	outb(0x00, addr+5); //spi start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) {
			break;
		}
		i++;
		if(i > 200) {
			printk(KERN_WARNING "reset_a %d, %d %d, %x %x\n",
				can_ch, tx_count, rx_count, kjg_val2[0], kjg_val2[2]);
			outb(0x00, addr); //clear
			return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) {
			break;
		}
		i++;
		if(i > 200) {
			printk(KERN_WARNING "reset_b %d, %d %d, %x %x %x %x\n",
				can_ch, tx_count, rx_count,
				kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);
			outb(0x00, addr); //clear
			return 0;
		}
	}
	/*printk(KERN_WARNING "reset_c %d, %d %d, %x %x %x %x\n",
		can_ch, tx_count, rx_count,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);*/

	//printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
	//	printf(" %02x", kjg_val[i]);
	}
	//printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
	//	printf("%02x ", kjg_val[i]);
	}
	//printf("\n");
	
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 6 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 1) i++;
	if(kjg_val[1] == 0x05 && kjg_val[2] == 0x01 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0x00 && kjg_val[5] == 0x00) i++;
	if(i == 2) return 1;
	else {
		printk(KERN_WARNING "reset_c %d, %d %d, %x %x %x %x, %02x %02x %02x %02x %02x\n",
			can_ch, tx_count, rx_count,
			kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
			kjg_val[1], kjg_val[2], kjg_val[3], kjg_val[4], kjg_val[5]);
		return 0;
	}
}

int rt_can_exec_cmd1(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd, kjg_val[80], kjg_val2[4];
	int addr, i;
	unsigned int ver;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[can_ch]);

	tx_count = 3;
	rx_count = 1 + 45;
	tx_cmd = 0x01; //get parameter
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	/*kjg_val2[0] = inb(addr); //spi_tx_index
	kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
	kjg_val2[2] = inb(addr+3); //fifo_status
	kjg_val2[3] = inb(addr+4); //rx_length
	printf("ch %d, status %d %d %d %d\n", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);*/

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	outb(0x00, addr+5); //spi start

	/*kjg_val2[0] = inb(addr); //spi_tx_index
	kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
	kjg_val2[2] = inb(addr+3); //fifo_status
	kjg_val2[3] = inb(addr+4); //rx_length
	printf("ch %d, status %d %d %d %d\n", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);*/

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) {
			break;
		}
		i++;
		if(i > 200) {
			printk(KERN_WARNING "cmd1_a %d, %d %d, %x %x\n", can_ch,
				tx_count, rx_count, kjg_val2[0], kjg_val2[2]);
			outb(0x00, addr); //clear
			return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) {
			break;
		}
		i++;
		if(i > 200) {
			printk(KERN_WARNING "cmd1_b %d, %d %d, %x %x %x %x\n",
				can_ch, tx_count, rx_count, 
				kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);
			outb(0x00, addr); //clear
			return 0;
		}
	}
	/*printk(KERN_WARNING "cmd1_c %d, %d %d, %x %x %x %x\n",
		can_ch, tx_count, rx_count,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);*/

	//printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
	//	printf(" %02x", kjg_val[i]);
	}
	//printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
	//	if(i <= 43) printf("%c", kjg_val[i]);
	//	else printf("%02x ", kjg_val[i]);
	//	if(i == 35 || i == 43) printf(", ");
	}
	//printf("\n");
	
	outb(0x00, addr); //clear

	/*kjg_val2[0] = inb(addr); //spi_tx_index
	kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
	kjg_val2[2] = inb(addr+3); //fifo_status
	kjg_val2[3] = inb(addr+4); //rx_length
	printf("ch%d, status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		//kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		if(i <= 43) printf("%c", kjg_val[i]);
		else printf("%02x ", kjg_val[i]);
		if(i == 35 || i == 43) printf(", ");
	}
	printf("\n");*/

	printk(KERN_WARNING "rt_can_fd: %d firmware_version %02x %02x %02x %02x %02x %02x %02x %02x\n",
		can_ch+1, kjg_val[36], kjg_val[37], kjg_val[38], kjg_val[39],
		kjg_val[40], kjg_val[41], kjg_val[42], kjg_val[43]);

	i = 0;
	if(kjg_val2[0] == 49 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 46) i++;
	if(kjg_val[1] == 3 && kjg_val[2] == 45 && kjg_val[3] == 1
		&& kjg_val[4] == 0x53 && kjg_val[5] == 0x59
		&& kjg_val[6] == 0x53 && kjg_val[7] == 0x54
		&& kjg_val[8] == 0x45 && kjg_val[9] == 0x4D
		&& kjg_val[10] == 0x5F && kjg_val[11] == 0x43
		&& kjg_val[12] == 0x41 && kjg_val[13] == 0x4E
		&& kjg_val[14] == 0x5F && kjg_val[15] == 0x46
		&& kjg_val[16] == 0x44 && kjg_val[17] == 0x5F
		&& kjg_val[18] == 0x44
		&& (kjg_val[19] >= 0x31 && kjg_val[19] <= 0x39)
		&& kjg_val[20] == 0x00
		&& (kjg_val[36] >= 0x32 && kjg_val[36] <= 0x34)
		&& (kjg_val[37] >= 0x30 && kjg_val[37] <= 0x39)
		&& (kjg_val[38] >= 0x30 && kjg_val[38] <= 0x31)
		&& (kjg_val[39] >= 0x30 && kjg_val[39] <= 0x39)
		&& (kjg_val[40] >= 0x30 && kjg_val[40] <= 0x33)
		&& (kjg_val[41] >= 0x30 && kjg_val[41] <= 0x39)
		&& (kjg_val[42] >= 0x30 && kjg_val[42] <= 0x39)
		&& (kjg_val[43] >= 0x30 && kjg_val[43] <= 0x39)
		) {
		i++;
		ver = ((unsigned int)(kjg_val[36] - 0x30) * 10000000);
		ver += ((unsigned int)(kjg_val[37] - 0x30) * 1000000);
		ver += ((unsigned int)(kjg_val[38] - 0x30) * 100000);
		ver += ((unsigned int)(kjg_val[39] - 0x30) * 10000);
		ver += ((unsigned int)(kjg_val[40] - 0x30) * 1000);
		ver += ((unsigned int)(kjg_val[41] - 0x30) * 100);
		ver += ((unsigned int)(kjg_val[42] - 0x30) * 10);
		ver += ((unsigned int)(kjg_val[43] - 0x30) * 1);
		p->can_fd_mcu_ver = ver;
	}
	if(i == 2) return 1;
	else return 0;
}

//shh_250114s
void printCanLogic(unsigned char bd, unsigned char *info, int infoSize)
//void printCanLogic(char bd, unsigned char info1, unsigned char info2, unsigned char info3)
{

	//CAN FD B TYPE - 03 : 41 30 33 //ttt
	//CAN FD B TYPE - 07 : 41 30 37
	//CAN FD B TYPE - 08 : 41 30 38
	//CAN FD C TYPE   	 : 41 35 30 
	//CAN FD D TYPE    	 : 44 30 38
	
	
	if(info[0] == 0x41 && info[1] == 0x30) {
	   info[2]  = info[2] & 0x0F; //37 -> 07, 38 -> 08
		printk(KERN_INFO "[pne_log] BD%d CAN FD B TYPE %x\n", bd, info[2]);
	} else if(info[0] == 0x41 && info[1] == 0x35 && info[2] == 0x30) {
		printk(KERN_INFO "[pne_log] BD%d CAN FD C TYPE\n", bd);
	} else if(info[0] == 0x44 && info[1] == 0x30 && info[2] == 0x38) {
		printk(KERN_INFO "[pne_log] BD%d CAN FD D TYPE\n",bd);
	} else if(info[0] == 0xff && info[1] == 0xff && info[2] == 0xff) {
		printk(KERN_INFO "[pne_warning] BD%d Can't find 'CAN BD'\n", bd);
	} else if(info[0] == 0x7f && info[1] == 0x3f && info[2] == 0x3f) {
		printk(KERN_INFO "[pne_warning] BD%d  BD Gender Error\n", bd);
	} else {
		printk(KERN_INFO "[pne_warning] BD%d Not defined Error\n", bd);
	}


}

void checkCanLogic(unsigned char *arr, int arrSize)
{
	//max_can_fd_ch => it is defined in rt_can.h

	if(max_can_fd_ch == 8) {
		printCanLogic(1,(unsigned char *)&arr[0], 3); //arr[0] ~ [2]
		printCanLogic(2,(unsigned char *)&arr[3], 3); //arr[3] ~ [5]
	} else {
		printCanLogic(1,(unsigned char *)&arr[0], 3); //arr[0] ~ [2]
	}
		
	/*	
	printk(KERN_INFO "[pne_log]rt_can_fd: board1 %x %x %x, board2 %x %x %x\n",
	arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
	*/

}
//shh_250114e

int init_rt_can(void)
{
	unsigned char info[6];
	int i, j, rtn;
	struct rt_can_fd_struct *p;

	terminal_resistor[0] = terminal_resistor[1] = 0x0F;

	max_can_fd_ch = 0;
	i = 0;

	info[0] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_1);
	info[1] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_2);
	info[2] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_3);
	//D,d,T,t (D : for production)
	if((info[0] == 0x44 || info[0] == 0x64
		|| info[0] == 0x54 || info[0] == 0x74)
		&& (info[1] >= 0x30 && info[1] <= 0x39)
		&& (info[2] >= 0x30 && info[2] <= 0x39)) {
		max_can_fd_ch = 4;
		i = 1;
	}

	info[3] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_1);
	info[4] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_2);
	info[5] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_3);
	//D,d,T,t (D : for production)
	if((info[3] == 0x44 || info[3] == 0x64
		|| info[3] == 0x54 || info[3] == 0x74)
		&& (info[4] >= 0x30 && info[4] <= 0x39)
		&& (info[5] >= 0x30 && info[5] <= 0x39)) {
		max_can_fd_ch += 4;
		i += 10;
	}

	checkCanLogic((unsigned char *)&info[0], 6); //shh_250114

	//kjg_240519 INT_MASK disable
	outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_OUT_MASK);
	outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_OUT_MASK);

	outb(0x0F, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_OUT_3); //CAN_RESET set
	outb(0x0F, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_OUT_3);
	//usleep(10);
	for(j=0; j < 200; j++) {}
	outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_OUT_3); //CAN_RESET clear
	outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_OUT_3);
	//usleep(10);
	for(j=0; j < 200; j++) {}

	printk(KERN_WARNING
		"rt_can_fd: bd1 %02x %02x %02x, bd2 %02x %02x %02x, %d\n",
		info[0], info[1], info[2], info[3], info[4], info[5], max_can_fd_ch);

    if((max_can_fd_ch == 4 && i == 1) || (max_can_fd_ch == 8 && i == 11)) {
		j = 0;
		/*for(i=0; i < max_can_fd_ch; i++) {
			rtn = 0;
			if(rt_can_exec_cmd_reset(i) > 0) rtn = rt_can_exec_cmd1(i);
			if(rtn > 0) j++;
		}*/
		for(i=0; i < max_can_fd_ch; i++) {
			rtn = rt_can_exec_cmd_reset(i);
			if(rtn != 1) j++;
		}
	} else j = 2;


	if(j != 0) {
		printk(KERN_WARNING "rt_can_fd: can't request all port regions %d %d %d\n",
			max_can_fd_ch, i, j);

    	return (-1);
	}

    for(i=0; i < max_can_fd_ch; i++) {
		p = &(rt_can_fd_table[i]);

		p->status = 0;
		p->set_phase = PH0;
		p->tmp_set_phase = PH0;

		p->rx_msg_write_idx = 0;
		p->rx_msg_read_idx = 0;
		p->rx_msg_count = 0;
		p->tx_msg_write_idx = 0;
		p->tx_msg_read_idx = 0;
		p->tx_msg_count = 0;
		p->tx_msg_error = 0;
		p->type = 0;

		p->chip_transfer_phase = PH0;
		p->chip_op_phase = PH0;
		p->chip_tx_rx_phase = PH0;
   	}

	printk(KERN_WARNING "rt_can_fd: driver loaded kjg_240624_1\n");

	return 0;
}

void cleanup_rt_can(void)
{
    int i;

    for(i=0; i < max_can_fd_ch; i++) {
		rt_can_exec_cmd_reset(i);
    }

	printk(KERN_WARNING "rt_can_fd: driver unloaded\n");
}
