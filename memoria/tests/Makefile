CC = gcc
CFLAGS = -I.. -I../.. -I../../ -pthread -Wall
LDFLAGS = -L../../utils/lib -lutils -pthread

TEST_SWAP = test_swap
TEST_SWAP_SRC = test_swap.c ../src/swap_space/swap_manager.c ../src/semaphores.c

all: $(TEST_SWAP)

$(TEST_SWAP): $(TEST_SWAP_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TEST_SWAP)

.PHONY: all clean
