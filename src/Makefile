ORACLE_HOME = /oracle/orahome
ORA_H = -I$(ORACLE_HOME)/rdbms/public
ORA_LIB_DIR = -L$(ORACLE_HOME)/lib
ORA_LIB = -lclntsh
UTL_DIR = ../../utils
UTL_H = -I../..
MBUNI_H = -I/home/mmsc/mmsc_src/mbuni-1.6.0
MBUNI_UTL_H = -I/home/mmsc/mmsc_src/mbuni-1.6.0/mmlib
KANNEL_H = -I/usr/local/include/kannel
LIBXML_H = -I/home/mmsc/mmsc_src/libxml2-2.7.6/include

mmsc-detokenizer: ./obj/mmsc-detokenizer.o ./obj/dbpool.o ./obj/detokenizer-auxiliary.o ./obj/config.o ./obj/log.o
	gcc -shared -o ../libmmsc-detokenizer.so ./obj/mmsc-detokenizer.o ./obj/dbpool.o ./obj/detokenizer-auxiliary.o ./obj/config.o ./obj/log.o -lstdc++ -lcurl $(ORA_LIB_DIR) $(ORA_LIB)
./obj/mmsc-detokenizer.o: mmsc-detokenizer.cpp ./obj/detokenizer-auxiliary.o ./obj/dbpool.o
	gcc -fPIC -c mmsc-detokenizer.cpp -o ./obj/mmsc-detokenizer.o $(UTL_H) $(ORA_H) $(MBUNI_H) $(MBUNI_UTL_H) $(KANNEL_H) $(LIBXML_H)
./obj/detokenizer-auxiliary.o: detokenizer-auxiliary.cpp
	g++ -fPIC -c detokenizer-auxiliary.cpp $(UTL_H) -o ./obj/detokenizer-auxiliary.o
./obj/dbpool.o: $(UTL_DIR)/dbpool/dbpool.cpp
	g++ -fPIC -c $(UTL_DIR)/dbpool/dbpool.cpp $(UTL_H) $(ORA_H) -o ./obj/dbpool.o
./obj/config.o: $(UTL_DIR)/config/config.cpp
	g++ -fPIC -c $(UTL_DIR)/config/config.cpp -o ./obj/config.o
./obj/log.o: $(UTL_DIR)/log/log.cpp
	g++ -fPIC -c $(UTL_DIR)/log/log.cpp -o ./obj/log.o

install:
	cp -f ../libmmsc-detokenizer.so /usr/local/lib

clean:
	rm -f ./obj/* ../libmmsc-detokenizer.so