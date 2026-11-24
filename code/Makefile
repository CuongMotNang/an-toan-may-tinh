# Makefile - Bài lab tràn bộ đệm (ĐH KHTN - Nhập môn An toàn máy tính)
# Dùng trên máy ảo SEED Ubuntu (cần gcc-multilib cho -m32).

STUDENT_ID ?= 23001842
CFLAGS      = -m32 -g -O0 -fno-stack-protector -z execstack

.PHONY: all setuid badfile disable-aslr run clean help

all: stack

# Biên dịch chương trình có lỗ hổng
stack: stack.c
	gcc $(CFLAGS) -o stack stack.c

# Gán quyền root (setuid) cho chương trình (cần sudo)
setuid: stack
	sudo chown root stack
	sudo chmod 4755 stack
	ls -l stack

# Tạo badfile từ exploit.py (nhớ chỉnh offset/ret trước)
badfile: exploit.py
	python3 exploit.py

# Tắt ASLR để địa chỉ stack ổn định khi debug & tấn công
disable-aslr:
	sudo sysctl -w kernel.randomize_va_space=0

# Chạy thử tấn công
run: badfile
	./stack $(STUDENT_ID)

# Dọn dẹp
clean:
	rm -f stack badfile

help:
	@echo "make           - bien dich stack"
	@echo "make setuid    - gan quyen root cho stack (can sudo)"
	@echo "make disable-aslr - tat ASLR"
	@echo "make badfile   - tao badfile tu exploit.py"
	@echo "make run STUDENT_ID=23001842 - chay thu"
	@echo "make clean     - xoa file bien dich"
