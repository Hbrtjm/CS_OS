CC = gcc
CFLAGS = -g -Wall -lpthread -Werror

SRCDIR = src
BLDDIR = build
OBJDIR = objects

TARGET = $(BLDDIR)/hospital_simulation

SRCS = common.c queue.c patient.c pharmacist.c doctor.c clinic.c main.c
OBJS = $(SRCS:.c=.o)
OBJPATHS = $(addprefix $(OBJDIR)/, $(OBJS))

.PHONY: all clean

all: $(TARGET) test

$(TARGET): $(OBJPATHS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR) $(BLDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

test:
	$(BLDDIR)/hospital_simulation

clean:
	rm -rf $(BLDDIR) $(OBJDIR)

