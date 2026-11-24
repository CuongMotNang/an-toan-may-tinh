# Giải thích nội dung kiến thức — Tấn công tràn bộ đệm (Buffer Overflow)

**Môn:** Nhập môn An toàn máy tính — Trường Đại học Khoa học Tự nhiên (HUS)
**Nhóm 3:** Lê Văn Cường (23001842), Nguyễn Việt Anh (23001829), Nguyễn Mai Hoàng Anh (23001824)

Tài liệu này giải thích đầy đủ các khái niệm cần nắm để hiểu và trình bày bài lab.

---

## 1. Bộ nhớ của một tiến trình

Khi một chương trình chạy, bộ nhớ ảo của nó được chia thành các vùng chính (từ địa chỉ thấp → cao):

| Vùng | Nội dung |
|------|----------|
| Text | Mã lệnh của chương trình (chỉ đọc) |
| Data / BSS | Biến toàn cục, biến tĩnh |
| Heap | Cấp phát động (`malloc`), lớn dần lên địa chỉ cao |
| **Stack** | Biến cục bộ, tham số, địa chỉ quay về; lớn dần xuống địa chỉ **thấp** |

Bài lab này tập trung vào **Stack**.

---

## 2. Khung ngăn xếp (Stack Frame) của một lời gọi hàm

Mỗi khi gọi một hàm, CPU tạo một "khung" trên stack. Với hàm `bof()`:

```
Địa chỉ THẤP
  ┌─────────────────────────┐  <- esp (đỉnh stack)
  │   buffer[ ]  (VLA)       │
  │   ... (vùng đệm) ...     │
  ├─────────────────────────┤
  │   saved ebp             │  <- ebp
  ├─────────────────────────┤
  │   RETURN ADDRESS        │  <- ebp + 4   (nơi CPU quay về sau hàm)
  ├─────────────────────────┤
  │   tham số hàm           │
  └─────────────────────────┘
Địa chỉ CAO
```

- **ebp** (base/frame pointer): mốc để truy cập biến cục bộ và tham số.
- **Return address**: địa chỉ lệnh kế tiếp trong hàm gọi; CPU nạp giá trị này vào `eip` khi hàm `bof()` thực hiện lệnh `ret`.
- `strcpy(buffer, str)` ghi dữ liệu **từ `buffer` đi lên phía địa chỉ cao** → nếu `str` dài, nó sẽ lần lượt đè lên saved `ebp` rồi đến **return address**.

> **Ý tưởng cốt lõi:** nếu ta kiểm soát được dữ liệu ghi tràn, ta có thể đặt một địa chỉ *tùy ý* vào ô return address → điều khiển luồng thực thi.

---

## 3. Lỗ hổng trong `stack.c`

```c
int bof(char *str, int bufsize) {
    char buffer[bufsize];
    strcpy(buffer, str);   // không kiểm tra độ dài -> TRÀN
    return 1;
}
```

- `str` được đọc 517 byte từ `badfile`.
- `buffer` chỉ 100–300 byte.
- `strcpy` sao chép đến khi gặp byte `\0`; vì ta không đặt `\0` sớm nên nó ghi tràn toàn bộ 517 byte.

### Điểm khác biệt: buffer ngẫu nhiên theo MSV

```c
unsigned int seed = strtoul(argv[1], NULL, 10);  // MSV
srand(seed);
int bufsize = BUF_LOW + rand() % (BUF_HIGH - BUF_LOW + 1);  // trong [100,300]
char buffer[bufsize];   // VLA -> địa chỉ & kích thước thay đổi
```

- Vì `buffer` là **mảng có độ dài thay đổi (VLA)**, cả **kích thước lẫn địa chỉ** của nó trên stack phụ thuộc `bufsize`.
- `bufsize` được quyết định bởi MSV ⇒ mỗi nhóm có một bố cục stack khác nhau ⇒ **offset tới return address khác nhau** ⇒ không thể dùng chung lời giải.
- `srand` + `rand` là **tất định (deterministic)**: cùng một MSV luôn cho cùng `bufsize` ⇒ nhóm có thể debug một lần và dùng lại.

---

## 4. Vũ khí: Shellcode

**Shellcode** là một đoạn mã máy ngắn, khi được thực thi sẽ mở một shell. Trong bài này dùng shellcode 32-bit gồm 2 phần:

1. `setuid(0)` — đặt UID hiệu dụng về 0 (root). Cần thiết vì nhiều shell (`/bin/sh` → `dash`) tự **hạ đặc quyền** nếu euid ≠ ruid; gọi `setuid(0)` trước giúp giữ root.
2. `execve("/bin/sh", NULL, NULL)` — thay tiến trình hiện tại bằng `/bin/sh`.

Vì chương trình `stack` có bit **setuid-root** (chạy với quyền root), shell sinh ra sẽ là **root shell**.

---

## 5. Kỹ thuật NOP sled

Vấn đề: ta không biết **chính xác** địa chỉ shellcode sẽ nằm ở đâu (có sai số nhỏ). Giải pháp:

- Lấp đầy phần đầu `badfile` bằng lệnh **NOP** (`0x90` — "không làm gì").
- Đặt shellcode ở **cuối**.
- Chọn return address (`ret`) trỏ vào **giữa vùng NOP**.

Khi CPU nhảy vào vùng NOP, nó "trượt" (sled) qua các lệnh NOP cho đến khi chạm shellcode. Điều này cho phép sai số lớn mà vẫn thành công.

### Cấu trúc `badfile` (517 byte)

```
[ 0x90 0x90 ... ][ RET (4 byte, little-endian) ][ 0x90 ... ][ shellcode ]
  ^offset = 0                 ^tại vị trí offset                  ^cuối file
```

---

## 6. Vì sao phải tắt các cơ chế bảo vệ?

Linux hiện đại có nhiều lớp phòng thủ. Để bài lab minh họa được, ta tắt chúng:

| Cơ chế | Vai trò | Cách tắt khi biên dịch/chạy |
|--------|---------|------------------------------|
| **ASLR** | Ngẫu nhiên hóa địa chỉ stack mỗi lần chạy | `sudo sysctl -w kernel.randomize_va_space=0` |
| **Stack Canary** | Giá trị kiểm tra đặt trước return address | `-fno-stack-protector` |
| **NX / DEP** | Cấm thực thi mã trên stack | `-z execstack` |
| 64-bit khó hơn | Địa chỉ 8 byte, thanh ghi khác | `-m32` (biên dịch 32-bit) |

> Trong thực tế, chính các cơ chế này (khi **bật**) là biện pháp phòng chống tràn bộ đệm.

---

## 7. Hai đại lượng mấu chốt cần tìm bằng gdb

1. **offset** — *nơi* đặt return address:
   
   `offset = (địa chỉ ô return address) − (địa chỉ buffer) = ($ebp + 4) − &buffer`

2. **ret** — *giá trị* ghi vào return address (trỏ vào NOP sled):
   
   `ret = &buffer + δ` (với δ là khoảng dư, ví dụ `0x64`)

Hai giá trị này được xác định cụ thể trong file [huong-dan-thuc-hien.md](huong-dan-thuc-hien.md).

---

## 8. Biện pháp phòng chống (trình bày ở phần kết luận)

- Dùng hàm sao chép có giới hạn: `strncpy`, `snprintf`, `strlcpy`.
- Luôn kiểm tra độ dài đầu vào.
- Bật Stack Canary, ASLR, NX (mặc định đã bật trên hệ thống hiện đại).
- Giảm thiểu chương trình setuid-root; áp dụng nguyên tắc đặc quyền tối thiểu.
- Dùng ngôn ngữ/an toàn bộ nhớ (Rust, Go) hoặc công cụ phân tích tĩnh.
