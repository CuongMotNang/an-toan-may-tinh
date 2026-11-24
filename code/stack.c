/* ============================================================================
 *  stack.c  --  Bài lab tràn bộ đệm (Buffer Overflow) - phiên bản MSV ngẫu nhiên
 *  Môn: Nhập môn An toàn máy tính - Trường Đại học Khoa học Tự nhiên (HUS)
 *
 *  Nhóm sinh viên:
 *    - Lê Văn Cường        - 23001842
 *    - Nguyễn Việt Anh     - 23001829
 *    - Nguyễn Mai Hoàng Anh - 23001824
 *
 *  Khác biệt so với chương trình trong bài giảng:
 *    (1) Kích thước (và do đó địa chỉ) của bộ đệm "buffer" KHÔNG cố định,
 *        mà được chọn NGẪU NHIÊN trong khoảng [BUF_LOW, BUF_HIGH] cho trước.
 *    (2) Chương trình nhận tham số dòng lệnh là Mã sinh viên (MSV) và dùng
 *        chính MSV làm MẦM (seed) cho bộ sinh số ngẫu nhiên.
 *        => Mỗi nhóm có một kích thước buffer riêng, do đó offset tới
 *           return address khác nhau, tránh trùng lời giải giữa các nhóm.
 *
 *  Cách chạy:  $ ./stack <MSV>        (ví dụ: ./stack 23001842)
 *
 *  Biên dịch trên máy ảo SEED Ubuntu (32-bit, tắt bảo vệ stack, cho phép
 *  thực thi mã trên stack), rồi gán quyền root (setuid):
 *      gcc -m32 -g -O0 -fno-stack-protector -z execstack -o stack stack.c
 *      sudo chown root stack
 *      sudo chmod 4755 stack
 * ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Khoảng (range) kích thước bộ đệm cho trước. Có thể đổi khi biên dịch:
 *   gcc ... -DBUF_LOW=120 -DBUF_HIGH=250 ...
 */
#ifndef BUF_LOW
#define BUF_LOW   100      /* cận dưới kích thước buffer (byte) */
#endif
#ifndef BUF_HIGH
#define BUF_HIGH  300      /* cận trên kích thước buffer (byte) */
#endif

/* Số byte đọc từ badfile */
#define INPUT_SIZE 517

/* ----------------------------------------------------------------------------
 *  Hàm chứa lỗ hổng. "buffer" là mảng có độ dài thay đổi (VLA): kích thước
 *  và vị trí của nó trên stack phụ thuộc vào bufsize (được quyết định bởi MSV).
 *  strcpy() KHÔNG kiểm tra độ dài nguồn -> nếu str dài hơn buffer sẽ gây
 *  TRÀN BỘ ĐỆM, ghi đè lên saved frame pointer và return address.
 * ----------------------------------------------------------------------------
 */
int bof(char *str, int bufsize)
{
    char buffer[bufsize];          /* VLA: địa chỉ & kích thước không cố định */
    strcpy(buffer, str);           /* <-- LỖ HỔNG: tràn bộ đệm */
    return 1;
}

int main(int argc, char **argv)
{
    char  str[INPUT_SIZE];
    FILE *badfile;

    if (argc < 2) {
        fprintf(stderr, "Cach dung: %s <MSV>\n", argv[0]);
        return 1;
    }

    /* Dùng MSV làm mầm gieo ngẫu nhiên (deterministic theo từng nhóm) */
    unsigned int seed = (unsigned int) strtoul(argv[1], NULL, 10);
    srand(seed);

    /* Kích thước buffer ngẫu nhiên trong khoảng [BUF_LOW, BUF_HIGH] */
    int bufsize = BUF_LOW + rand() % (BUF_HIGH - BUF_LOW + 1);

    printf("MSV (seed)        : %u\n", seed);
    printf("Khoang buffer     : [%d, %d]\n", BUF_LOW, BUF_HIGH);
    printf("Kich thuoc buffer : %d byte\n", bufsize);

    /* Đọc dữ liệu tấn công từ badfile */
    badfile = fopen("badfile", "r");
    if (badfile == NULL) {
        fprintf(stderr, "Khong mo duoc 'badfile'. Hay chay exploit.py truoc.\n");
        return 1;
    }
    fread(str, sizeof(char), INPUT_SIZE, badfile);
    fclose(badfile);

    bof(str, bufsize);

    printf("Returned Properly\n");   /* Dòng này KHÔNG in ra nếu tấn công thành công */
    return 1;
}
