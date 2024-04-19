.PHONY: all clean

SRC = read_boot.c read_file.c read_first_partition.c read_mbr.c read_root.c recovery_file.c 
BIN = $(SRC:.c=)

clean:
	rm -f $(BIN) $(OBJ)

all: 
	gcc read_boot.c -o read_boot
	gcc read_file.c -o read_file
	gcc read_first_partition.c -o read_first_partition
	gcc read_mbr.c -o  read_mbr
	gcc read_root.c -o read_root
	gcc recovery_file.c -o recovery_file
