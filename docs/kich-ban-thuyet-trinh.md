# Kịch bản thuyết trình (10 phút) — theo từng slide

**Môn:** Nhập môn An toàn máy tính — HUS
**Nhóm 3:** Lê Văn Cường (23001842), Nguyễn Việt Anh (23001829), Nguyễn Mai Hoàng Anh (23001824)

> Tổng thời lượng mục tiêu: **10 phút**. Cột thời gian là gợi ý. Phần trong ngoặc `(…)` là ghi chú thao tác, không đọc.

---

### Slide 1 — Trang bìa *(0:00–0:20)*
> "Em xin chào thầy/cô và các bạn. Nhóm 3 chúng em gồm Lê Văn Cường, Nguyễn Việt Anh và Nguyễn Mai Hoàng Anh xin trình bày bài tập lớn: *Khai thác lỗ hổng tràn bộ đệm để đạt root shell*, môn Nhập môn An toàn máy tính."

### Slide 2 — Nội dung *(0:20–0:35)*
> "Bài trình bày gồm: phân công công việc, bài toán và ý tưởng, kiến thức nền tảng, các bước thực hiện tấn công, khai báo sử dụng AI và kết luận."

### Slide 3 — Phân công công việc *(0:35–1:05)*
> "Về phân công: bạn Cường phụ trách chương trình `stack.c`, thiết lập môi trường và debug gdb để tìm offset. Bạn Việt Anh phụ trách `exploit.py`, tính địa chỉ return address và quay video. Bạn Hoàng Anh soạn slide, tài liệu, vẽ sơ đồ và phần khai báo AI. Cả nhóm cùng kiểm chứng kết quả cuối."

### Slide 4 — Đề bài và điểm khác biệt *(1:05–2:00)*
> "Chương trình `stack.c` có lỗ hổng tràn bộ đệm do dùng `strcpy`. Điểm khác so với bài giảng là: thứ nhất, kích thước và địa chỉ bộ đệm không cố định mà nằm ngẫu nhiên trong khoảng 100 đến 300 byte; thứ hai, chương trình nhận mã sinh viên làm tham số và dùng nó làm mầm gieo ngẫu nhiên. Nhờ vậy mỗi nhóm có một kích thước buffer riêng, nên offset tới return address khác nhau và lời giải là duy nhất. Mục tiêu của nhóm là biên dịch chương trình thành tiến trình đặc quyền root rồi khai thác để đạt root shell."

### Slide 5 — Bố cục bộ nhớ & stack frame *(2:00–3:15)*
> "Để hiểu cách tấn công, ta xem khung ngăn xếp khi gọi hàm `bof`. Trên stack, từ địa chỉ thấp lên cao lần lượt là: mảng `buffer`, saved ebp, rồi đến **return address** — là địa chỉ CPU sẽ quay về sau khi hàm kết thúc. Quan trọng là `strcpy` ghi dữ liệu từ `buffer` đi lên phía địa chỉ cao, nên nếu dữ liệu đủ dài, nó sẽ đè lên return address. Đây chính là chỗ chúng em khai thác."

### Slide 6 — Lỗ hổng trong stack.c *(3:15–4:00)*
> "Đây là hàm chứa lỗ hổng. `buffer` chỉ 100 đến 199 byte, nhưng `str` đọc tới 512 byte từ badfile, và `strcpy` không kiểm tra độ dài. Vì vậy nếu nhóm đặt đúng một địa chỉ vào vị trí return address, CPU sẽ nhảy tới nơi nhóm muốn khi hàm kết thúc."

### Slide 7 — Cấu trúc badfile *(4:00–4:50)*
> "Ý tưởng tấn công: badfile gồm một dãy NOP — các lệnh không làm gì — ở đầu, địa chỉ ghi đè ở vị trí return address, và shellcode đặt ở cuối. Nhóm cho return address trỏ vào giữa vùng NOP; CPU nhảy vào đó sẽ trượt qua các NOP cho tới khi gặp shellcode. Shellcode gồm `setuid(0)` để giữ quyền root, rồi `execve` gọi `/bin/sh`."

### Slide 8 — Môi trường & biên dịch *(4:50–5:45)*
> "Nhóm thực hiện trên máy ảo SEED Ubuntu 32-bit. Trước hết tắt ASLR để địa chỉ stack ổn định. Biên dịch với `-m32`, `-fno-stack-protector` để tắt stack canary, và `-z execstack` để cho phép thực thi mã trên stack. Cuối cùng `chown root` và `chmod 4755` để bật bit setuid, giúp chương trình chạy với quyền root." *(Nhấn mạnh: các cờ này khi bật lại chính là biện pháp phòng chống.)*

### Slide 9 — Bước 1: NƠI đặt return address *(5:45–7:00)*
> "Bước một: xác định *nơi* đặt return address. Nhóm dùng gdb, dừng tại dòng `strcpy`, chạy với mã sinh viên của nhóm. Khi dừng, nhóm in giá trị `$ebp` và địa chỉ `buffer` (con trỏ alloca). Return address nằm ngay trên saved ebp, tức tại `$ebp + 4`. Offset bằng `$ebp + 4` trừ đi địa chỉ buffer." *(Chỉ vào ảnh chụp gdb, đọc số liệu thực tế của nhóm.)*

### Slide 10 — Bước 2: GIÁ TRỊ return address *(7:00–7:50)*
> "Bước hai: xác định *giá trị* return address. Nhóm lấy địa chỉ `buffer` từ gdb, rồi cộng thêm một khoảng dư để trỏ vào giữa vùng NOP. Vì đã tắt ASLR nên địa chỉ trong gdb trùng với khi chạy thật." *(Chỉ vào ảnh và đọc con số cụ thể.)*

### Slide 11 — Bước 3: Chỉnh exploit.py *(7:50–8:40)*
> "Bước ba: đưa hai giá trị `offset` và `ret` vừa tìm được vào `exploit.py`. Chương trình lấp đầy badfile bằng NOP, đặt shellcode ở cuối, và ghi địa chỉ `ret` dạng little-endian vào vị trí offset. Chạy `python3 exploit.py` để sinh badfile 512 byte."

### Slide 12 — Bước 4: Thực hiện tấn công *(8:40–9:20)*
> "Bước bốn: chạy `./stack` với mã sinh viên. Dấu nhắc đổi thành dấu thăng, lệnh `id` cho `uid=0(root)` — tức nhóm đã có root shell. Dòng 'Returned Properly' không xuất hiện, chứng tỏ luồng thực thi đã bị chuyển hướng thành công."

### Slide 13 — Sơ đồ quy trình *(9:20–9:40)*
> "Tóm lại toàn bộ quy trình: biên dịch và setuid → dùng gdb tìm địa chỉ → tính offset và ret → sửa exploit.py tạo badfile → chạy chương trình → nhận root shell."

### Slide 14 — Khai báo sử dụng AI *(9:40–9:55)*
> "Nhóm xin khai báo rõ việc dùng AI theo ba mức: Mức 1 — dùng AI để tranh luận, phản biện về cách tính offset, sau đó nhóm tự triển khai. Mức 2 — AI đồng sáng tạo khung code và slide theo thiết kế của nhóm, nhóm biên tập và chịu trách nhiệm. Mức 3 — AI sinh nội dung tài liệu theo prompt của nhóm, nhóm ghép nối và kiểm chứng. **Toàn bộ số liệu địa chỉ và kết quả tấn công đều do nhóm tự thực hiện trên máy của mình.**"

### Slide 15 + 16 — Kết luận & cảm ơn *(9:55–10:00)*
> "Nhóm đã khai thác thành công lỗ hổng tràn bộ đệm để đạt root shell. Biện pháp phòng chống gồm: dùng hàm an toàn, bật stack canary, ASLR, NX và hạn chế chương trình setuid. Cảm ơn thầy/cô và các bạn đã lắng nghe."

---

## Gợi ý phân vai nói
- **Việt Anh:** slide 1–4 (dẫn nhập, đề bài).
- **Hoàng Anh:** slide 5–8 (kiến thức nền, môi trường).
- **Cường:** slide 9–13 (demo kỹ thuật, gdb, tấn công).
- **Cả nhóm:** slide 14–16 (khai báo AI, kết luận) — mỗi người một ý.
