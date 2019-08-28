#
# J1939Display Makefile
#

CC = gcc
LD = gcc

TCAN_API_DIR = /opt/tiny_can/dev/lib

OBJ_DIR = obj
OUTPUT_DIR = bin
PROGRAMM = ObdDisplay

SRC_DIR = ./../src
RES_DIR = ./../res
C_PREPROC = -DENABLE_NLS

CFLAGS = `pkg-config --cflags gtk+-2.0 libxml-2.0` -Wall \
         -I$(SRC_DIR) -I$(SRC_DIR)/lib -I$(SRC_DIR)/LinuxLib -I$(SRC_DIR)/obd -I$(SRC_DIR)/www \
         -I$(TCAN_API_DIR) -pthread -g0 -O0 #-g0 -O2 ggdb3

LIBS = `pkg-config --libs gtk+-2.0 libxml-2.0` -ldl -lm
LDFLAGS = -pthread


SRC_OBJS = \
  $(OBJ_DIR)/mhs_event.o \
  $(OBJ_DIR)/mhs_thread.o \
  $(OBJ_DIR)/can_autobaud.o \
  $(OBJ_DIR)/lcd_gauge_win.o \
  $(OBJ_DIR)/cbuf.o \
  $(OBJ_DIR)/configfile.o \
  $(OBJ_DIR)/etable.o \
  $(OBJ_DIR)/gauge-private.o \
  $(OBJ_DIR)/gauge-xml.o \
  $(OBJ_DIR)/gauge.o \
  $(OBJ_DIR)/gtk-digital-show.o \
  $(OBJ_DIR)/gtk-ex-frame.o \
  $(OBJ_DIR)/gtk-lcd.o \
  $(OBJ_DIR)/gtk_util.o \
  $(OBJ_DIR)/iconcache.o \
  $(OBJ_DIR)/mhs_g_event.o \
  $(OBJ_DIR)/mhs_g_messages.o \
  $(OBJ_DIR)/mhs_queue.o \
  $(OBJ_DIR)/msg_widget.o \
  $(OBJ_DIR)/util.o \
  $(OBJ_DIR)/xmlbase.o \
  $(OBJ_DIR)/main.o \
  $(OBJ_DIR)/can_dev_pnp.o \
  $(OBJ_DIR)/can_device.o \
  $(OBJ_DIR)/dtc_db.o \
  $(OBJ_DIR)/isotp.o \
  $(OBJ_DIR)/obd2.o \
  $(OBJ_DIR)/obd_db.o \
  $(OBJ_DIR)/vin_db.o  \
  $(OBJ_DIR)/obd2_dtc_win.o \
  $(OBJ_DIR)/obd2_list_win.o \
  $(OBJ_DIR)/obd2_pid_sel.o \
  $(OBJ_DIR)/setup.o \
  $(OBJ_DIR)/start_win.o \
  $(OBJ_DIR)/trace.o \
  $(OBJ_DIR)/vin_info_win.o \
  $(OBJ_DIR)/vin_wdg.o \
  $(OBJ_DIR)/xml_database.o \
  $(OBJ_DIR)/sock_lib.o \
  $(OBJ_DIR)/open_xc.o \
  $(OBJ_DIR)/mhs_file_event.o \
  $(OBJ_DIR)/can_drv_linux.o 

all: create_dirs create_app

create_dirs:
	mkdir -p "$(OBJ_DIR)"
	mkdir -p "$(OUTPUT_DIR)"
	cp -p $(RES_DIR)/* $(OUTPUT_DIR) 

create_app: $(SRC_OBJS)
	$(LD) -o $(OUTPUT_DIR)/$(PROGRAMM) $(CFLAGS) $(SRC_OBJS) $(LDFLAGS) $(LIBS)


$(OBJ_DIR)/mhs_event.o: $(SRC_DIR)/LinuxLib/mhs_event.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/mhs_thread.o: $(SRC_DIR)/LinuxLib/mhs_thread.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/mhs_file_event.o: $(SRC_DIR)/LinuxLib/mhs_file_event.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@  

$(OBJ_DIR)/can_autobaud.o: $(SRC_DIR)/can_autobaud.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@    

$(OBJ_DIR)/lcd_gauge_win.o: $(SRC_DIR)/lcd_gauge_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@  

$(OBJ_DIR)/cbuf.o: $(SRC_DIR)/lib/cbuf.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/configfile.o: $(SRC_DIR)/lib/configfile.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/etable.o: $(SRC_DIR)/lib/etable.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gauge-private.o: $(SRC_DIR)/lib/gauge-private.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gauge-xml.o: $(SRC_DIR)/lib/gauge-xml.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gauge.o: $(SRC_DIR)/lib/gauge.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk-digital-show.o: $(SRC_DIR)/lib/gtk-digital-show.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk-ex-frame.o: $(SRC_DIR)/lib/gtk-ex-frame.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk-lcd.o: $(SRC_DIR)/lib/gtk-lcd.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk_util.o: $(SRC_DIR)/lib/gtk_util.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@  

$(OBJ_DIR)/iconcache.o: $(SRC_DIR)/lib/iconcache.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@  

$(OBJ_DIR)/mhs_g_event.o: $(SRC_DIR)/lib/mhs_g_event.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@    

$(OBJ_DIR)/mhs_g_messages.o: $(SRC_DIR)/lib/mhs_g_messages.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/mhs_queue.o: $(SRC_DIR)/lib/mhs_queue.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/msg_widget.o: $(SRC_DIR)/lib/msg_widget.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/util.o: $(SRC_DIR)/lib/util.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/xmlbase.o: $(SRC_DIR)/lib/xmlbase.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_dev_pnp.o: $(SRC_DIR)/obd/can_dev_pnp.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_device.o: $(SRC_DIR)/obd/can_device.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/dtc_db.o: $(SRC_DIR)/obd/dtc_db.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/isotp.o: $(SRC_DIR)/obd/isotp.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/obd2.o: $(SRC_DIR)/obd/obd2.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/obd_db.o: $(SRC_DIR)/obd/obd_db.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/vin_db.o: $(SRC_DIR)/obd/vin_db.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/obd2_dtc_win.o: $(SRC_DIR)/obd2_dtc_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@  

$(OBJ_DIR)/obd2_list_win.o: $(SRC_DIR)/obd2_list_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/obd2_pid_sel.o: $(SRC_DIR)/obd2_pid_sel.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/setup.o: $(SRC_DIR)/setup.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/start_win.o: $(SRC_DIR)/start_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/trace.o: $(SRC_DIR)/trace.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/vin_info_win.o: $(SRC_DIR)/vin_info_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@  

$(OBJ_DIR)/vin_wdg.o: $(SRC_DIR)/vin_wdg.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/xml_database.o: $(SRC_DIR)/www/xml_database.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/sock_lib.o: $(SRC_DIR)/www/sock_lib.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@   

$(OBJ_DIR)/open_xc.o: $(SRC_DIR)/www/open_xc.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@      

$(OBJ_DIR)/can_drv_linux.o: $(TCAN_API_DIR)/can_drv_linux.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -r bin
	rm -r obj	
	rm -f *.o 
	rm -f *~ 
	rm -f ./$(OBJ_DIR)/*.o
	rm -f ./$(OBJ_DIR)/*~

