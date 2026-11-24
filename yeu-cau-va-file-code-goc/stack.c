/* stack.c
 * Biên dịch chương trình thành setuid root privileged program
 * Chạy chương trình này với tham số là Mã sinh viên của bạn
 *		./stack <MSV>
 * Yêu cầu: chương trình trả về một root shell (#)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>

/* Hàm dính lỗi Buffer Overflow */
int bof(char *str, int buffer_size)
{
    // Cấp phát động mảng trên Stack dựa trên kích thước cá nhân hóa
    char *buffer = (char *)alloca(buffer_size);
    
    strcpy(buffer, str); // Lỗi tràn bộ nhớ xảy ra tại đây
    return 1;
}

int main(int argc, char **argv)
{
    char str[512];
    FILE *badfile;
    long long mssv = 0;

    // Yêu cầu nhập MSSV để cá nhân hóa môi trường bài lab
    if (argc < 2) {
        printf("Sử dụng: %s <Mã_Số_Sinh_Viên>\n", argv[0]);
        printf("Ví dụ: %s 20261234\n", argv[0]);
        exit(1);
    }
    
    mssv = atoll(argv[1]);
    if (mssv <= 0) {
        printf("[LỖI] Mã số sinh viên không hợp lệ!\n");
        exit(1);
    }

    // 1. Tính toán Kích thước Buffer độc nhất cho từng MSSV
    // Khoảng nền: 100 bytes. Khoảng biến thiên: 0 - 99 bytes (Dựa vào MSSV)
    int base_buf = 100;
    int var_buf = mssv % 100; 
    int student_buffer_size = base_buf + var_buf;

    // 2. Tính toán Khoảng lệch Địa chỉ Stack độc nhất cho từng MSSV
    // Khoảng lệch tối đa là 256 bytes
    int student_stack_offset = (mssv * 7 + 13) % 257; 
    
    // Ép Stack dịch chuyển theo cấu trúc cá nhân
    char *dummy = (char *)alloca(student_stack_offset);
    if (dummy > 0) dummy[0] = '\0'; // Tránh Compiler tối ưu xóa bỏ

    // Đọc payload từ file "badfile" của sinh viên
    badfile = fopen("badfile", "r");
    if (!badfile) {
        perror("[LỖI] Không tìm thấy file 'badfile'. Vui lòng tạo payload trước.");
        exit(1);
    }
    int bytes_read = fread(str, sizeof(char), 512, badfile);
    fclose(badfile);

    // Thông báo thông tin (Dạng khoảng ước lượng) cho sinh viên tư duy
    printf("[HỆ THỐNG] Đang thiết lập không gian bộ nhớ cho MSSV: %lld\n", mssv);
    printf("[HỆ THỐNG] Kích thước buffer của bạn nằm trong khoảng [%d - %d] bytes.\n", base_buf, base_buf + 99);
    printf("[HỆ THỐNG] Khoảng lệch vị trí Stack của bạn nằm trong phạm vi [0 - 256] bytes.\n");
    printf("[HỆ THỐNG] Payload đọc vào: %d bytes. Đang thực thi hàm bof()...\n", bytes_read);

    bof(str, student_buffer_size);

    printf("Chương trình kết thúc bình thường (Tấn công thất bại).\n");
    return 0;
}
