# Bài tập lớn — Tấn công Tràn bộ đệm (Buffer Overflow)

**Môn:** Nhập môn An toàn máy tính — Trường Đại học Khoa học Tự nhiên (HUS)

**Nhóm 3**
| Thành viên | MSV |
|------------|-----|
| Lê Văn Cường | 23001842 |
| Nguyễn Việt Anh | 23001829 |
| Nguyễn Mai Hoàng Anh | 23001824 |

## Mục tiêu
Biên dịch `stack.c` (có lỗ hổng tràn bộ đệm, kích thước buffer ngẫu nhiên theo **Mã sinh viên**) thành chương trình **setuid-root**, rồi khai thác để đạt **root shell**.

## Cấu trúc thư mục
```
.
├── code/
│   ├── stack.c        # chương trình có lỗ hổng (buffer ngẫu nhiên theo MSV)
│   ├── exploit.py     # tạo badfile khai thác (điền offset/ret từ gdb)
│   └── Makefile       # biên dịch, setuid, tắt ASLR, chạy thử
├── slides/
│   ├── main.tex       # slide báo cáo (LaTeX Beamer, pdfLaTeX, tiếng Việt)
│   └── images/        # nơi đặt ảnh chụp màn hình gdb/tấn công
└── docs/
    ├── giai-thich-kien-thuc.md   # giải thích kiến thức
    ├── kich-ban-thuyet-trinh.md  # kịch bản thuyết trình từng slide
    ├── huong-dan-thuc-hien.md    # hướng dẫn chạy & cách tính offset/ret
    └── phan-cong-cong-viec.md    # phân công nhóm + khai báo AI
```

## Bắt đầu nhanh (trên máy ảo SEED Ubuntu 32-bit)
```bash
sudo sysctl -w kernel.randomize_va_space=0      # tắt ASLR
cd code
make && make setuid                             # biên dịch + setuid-root
# ... dùng gdb tìm offset/ret (xem docs/huong-dan-thuc-hien.md) ...
# ... điền offset/ret vào exploit.py ...
python3 exploit.py                              # tạo badfile
./stack 23001842                                # tấn công -> # (root)
```

Xem chi tiết từng bước và cách tính toán trong [docs/huong-dan-thuc-hien.md](docs/huong-dan-thuc-hien.md).

## Yêu cầu môi trường
- Linux 32-bit (SEED Ubuntu khuyến nghị) với `gcc-multilib`, `gdb`, `python3`.
- Slide biên dịch bằng **pdfLaTeX** (Overleaf free).

## Khai báo sử dụng AI
Nhóm có sử dụng AI hỗ trợ; mức độ áp dụng được khai báo rõ trong [docs/phan-cong-cong-viec.md](docs/phan-cong-cong-viec.md) và slide 14. Mọi số liệu gdb và kết quả tấn công do nhóm tự thực hiện, kiểm chứng.

> ⚠️ Mã nguồn chỉ dùng cho mục đích **học tập** trong môi trường thí nghiệm có kiểm soát.
