# Phân công công việc nhóm

**Môn:** Nhập môn An toàn máy tính — Trường Đại học Khoa học Tự nhiên (HUS)
**Đề tài:** Khai thác lỗ hổng tràn bộ đệm (Buffer Overflow) để đạt root shell.

---

## Thành viên & nhiệm vụ

| Thành viên | MSV | Nhiệm vụ chính | Sản phẩm phụ trách |
|------------|-----|----------------|--------------------|
| **Lê Văn Cường** | 23001842 | Phụ trách mã nguồn `stack.c`; thiết lập môi trường (biên dịch 32-bit, setuid, tắt ASLR); dùng **gdb** xác định offset tới return address. | `code/stack.c`, `code/Makefile`, ảnh gdb bước 1 |
| **Nguyễn Việt Anh** | 23001829 | Phụ trách `exploit.py`; tính **giá trị return address**, dựng badfile; **quay video** và lồng tiếng demo tấn công. | `code/exploit.py`, badfile, video YouTube |
| **Nguyễn Mai Hoàng Anh** | 23001824 | Soạn **slide** + tài liệu; vẽ sơ đồ (stack frame, badfile, quy trình); tổng hợp ảnh chụp màn hình; viết phần **khai báo sử dụng AI**. | `slides/main.tex`, tài liệu trong `docs/` |

> Cả ba thành viên **cùng rà soát** kết quả cuối và **chịu trách nhiệm chung** về toàn bộ nội dung nộp.

---

## Phân vai thuyết trình (10 phút)

| Thời lượng | Slide | Người trình bày |
|-----------|-------|-----------------|
| 0:00–2:00 | 1–4 (dẫn nhập, đề bài) | Nguyễn Việt Anh |
| 2:00–5:45 | 5–8 (kiến thức nền, môi trường) | Nguyễn Mai Hoàng Anh |
| 5:45–9:20 | 9–13 (demo gdb, tấn công, sơ đồ) | Lê Văn Cường |
| 9:20–10:00 | 14–16 (khai báo AI, kết luận) | Cả nhóm |

---

## Dòng thời gian thực hiện (gợi ý)

1. **Tuần 1:** Cường dựng `stack.c` + môi trường; Việt Anh dựng khung `exploit.py`.
2. **Tuần 2:** Cả nhóm debug gdb, tìm offset/ret, hoàn thiện badfile, tấn công thành công.
3. **Tuần 3:** Hoàng Anh hoàn thiện slide + tài liệu; Việt Anh quay & dựng video; cả nhóm rà soát và nộp.

---

## Khai báo mức độ sử dụng AI

Nhóm sử dụng AI hỗ trợ và khai báo rõ theo 3 mức (chi tiết ở slide 14):

1. **Mức 1 — Tranh luận & phản biện:** AI gợi ý/phản biện cách tính offset và chọn `ret`; **sinh viên tự triển khai** thao tác gdb và tính toán.
2. **Mức 2 — Đồng sáng tạo theo thiết kế của sinh viên:** AI hỗ trợ dựng khung `stack.c`, `exploit.py`, bố cục slide theo ý tưởng nhóm; **sinh viên biên tập, chỉnh sửa và chịu trách nhiệm** nội dung.
3. **Mức 3 — AI tạo sinh theo prompt:** AI sinh nội dung tài liệu giải thích và kịch bản thuyết trình theo prompt; **sinh viên viết prompt, ghép nối và kiểm chứng** mọi số liệu.

> **Phần do nhóm tự làm hoàn toàn:** mọi địa chỉ, offset, giá trị return address và kết quả tấn công (root shell) đều được nhóm tự thực hiện và kiểm chứng trên máy của nhóm, thể hiện trong video.
