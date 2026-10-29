# Khai báo môi trường và thao tác đọc/ghi file trong chương trình C


## 1. Môi trường sử dụng

### Phần mềm và nền tảng

- Hệ điều hành: Ubuntu/Linux.
- Trình biên dịch: GCC.
- Ngôn ngữ: C, chuẩn C11.
- File dữ liệu: `students.txt`.
- Tệp mã nguồn ví dụ: `demo.c`.
- Tên chương trình sau khi biên dịch: `student`.

Chương trình được biên dịch và khởi chạy bằng:

```bash
gcc -std=c11 -Wall -Wextra demo.c -o student
./student
```

Trong đó:

- `-std=c11` chọn chuẩn ngôn ngữ C11.
- `-Wall -Wextra` bật các cảnh báo hữu ích để phát hiện một số lỗi khi biên dịch.
- `demo.c` là tệp mã nguồn đầu vào.
- `-o student` đặt tên tệp thực thi là `student`.
- `./student` chạy chương trình ngay tại thư mục hiện tại.

### Tổ chức tệp

Đặt tệp dữ liệu cùng thư mục chạy chương trình để tên tương đối `students.txt` được tìm thấy đúng:

```text
thu_muc_du_an/
├── demo.c
└── students.txt
```

Nếu chương trình chạy từ thư mục khác, đường dẫn tương đối có thể trỏ đến vị trí khác. Khi báo lỗi không mở được file, hãy kiểm tra thư mục hiện tại bằng `pwd` và kiểm tra file bằng `ls -l students.txt`.

## 2. Dữ liệu chương trình xử lý

Một bản ghi điểm trong ví dụ gồm các trường:

| Trường | Ý nghĩa |
|---|---|
| `TT` | Số thứ tự |
| `SBD` | Số báo danh; nên lưu dưới dạng chuỗi để giữ số 0 ở đầu |
| `Họ và tên` | Tên thí sinh |
| `Tên trường` | Trường của thí sinh |
| `Tên môn` | Môn thi |
| `Bài 1`, `Bài 2`, `Bài 3` | Điểm từng bài |
| `Tổng` | Tổng điểm ba bài |

Ví dụ dữ liệu được nhập:

```text
TT: 89
SBD: 123456
Họ và tên: Phan Thanh Dũng
Tên trường: HUS
Tên môn: Toán học
Điểm bài 1: 9
Điểm bài 2: 9.5
Điểm bài 3: 8.5
```

Tổng điểm được tính như sau:

```text
Tổng = Điểm bài 1 + Điểm bài 2 + Điểm bài 3
     = 9 + 9.5 + 8.5
     = 27.0
```

## 3. Khai báo trong chương trình C

Để thao tác với file văn bản, chương trình cần khai báo thư viện chuẩn `stdio.h`. Thư viện này cung cấp `FILE`, `fopen`, `fclose`, `fscanf`, `fprintf`, `fgets` và `fputs`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

- `stdio.h`: nhập xuất màn hình và đọc/ghi file.
- `stdlib.h`: các tiện ích chung như chuyển đổi và kiểm tra giá trị trả về khi cần.
- `string.h`: xử lý chuỗi như `strcspn` để loại ký tự xuống dòng.

Có thể biểu diễn một bản ghi bằng `struct`. SBD được để dạng chuỗi nhằm giữ được số 0 ở đầu nếu có:

```c
#define TEXT_LEN 128

typedef struct {
    int thu_tu;
    char sbd[TEXT_LEN];
    char ho_ten[TEXT_LEN];
    char ten_truong[TEXT_LEN];
    char ten_mon[TEXT_LEN];
    double bai1;
    double bai2;
    double bai3;
    double tong;
} Student;
```

Tên trường và họ tên có thể chứa dấu cách, vì vậy khi nhập từ bàn phím nên dùng `fgets` thay cho `%s`. Khi đọc file, định dạng lưu phải được thiết kế rõ ràng để phân biệt các trường có dấu cách.

## 4. Cách chương trình thực hiện

Chương trình lặp menu cho đến khi người dùng chọn thoát:

```text
===== FILE I/O =====
1. Đọc và hiển thị students.txt
2. Nhập và ghi thêm dữ liệu vào students.txt
0. Thoát
Chọn:
```

Luồng xử lý tổng quát:

1. In menu.
2. Đọc lựa chọn của người dùng.
3. Nếu chọn `1`, mở file để đọc, lần lượt lấy các bản ghi và in thành bảng.
4. Nếu chọn `2`, nhập các trường của thí sinh, tính tổng điểm rồi mở file ở chế độ thêm để ghi bản ghi mới.
5. Nếu chọn `0`, kết thúc chương trình.
6. Nếu lựa chọn không hợp lệ, thông báo và hiển thị lại menu.

### Ảnh minh họa: đọc và hiển thị danh sách

Ảnh dưới đây ghi lại lần chạy chọn chức năng `1`: chương trình đọc dữ liệu từ `students.txt` và trình bày các thí sinh thành bảng.

![Chương trình đọc và hiển thị danh sách thí sinh](images/read_students.png)

## 5. Cách đọc file

### Mở file ở chế độ đọc

```c
FILE *file = fopen("students.txt", "r");
if (file == NULL) {
    perror("Khong mo duoc students.txt");
    return;
}
```

Chế độ `"r"` mở file để đọc. File cần tồn tại; nếu không, `fopen` trả về `NULL`. Kiểm tra con trỏ trước khi dùng giúp tránh lỗi truy cập bộ nhớ.

### Đọc nội dung

Cách đọc cụ thể phụ thuộc vào định dạng dữ liệu đang lưu trong `students.txt`.

- Nếu mỗi dòng là một trường đơn giản, có thể đọc từng dòng bằng `fgets` rồi phân tích.
- Nếu các trường được phân cách bằng dấu `;` hoặc ký tự tab, có thể tách từng dòng theo dấu phân cách.
- `fscanf` phù hợp với dữ liệu có định dạng cố định và không có khoảng trắng trong các trường dạng chuỗi. Không nên dùng `%s` để đọc họ tên hoặc tên trường có dấu cách vì nó chỉ lấy đến khoảng trắng đầu tiên.

Ví dụ đọc từng dòng an toàn hơn về mặt xử lý khoảng trắng:

```c
char line[512];
while (fgets(line, sizeof line, file) != NULL) {
    /* Phân tích line theo định dạng đã quy ước trong students.txt. */
    fputs(line, stdout);
}
```

Sau khi đọc xong, cần đóng file:

```c
fclose(file);
```

Chương trình của bạn sau đó trình bày các trường theo cột `TT`, `SBD`, họ tên, trường, môn, điểm từng bài và tổng điểm. Việc căn cột là phần định dạng đầu ra; dữ liệu được lấy từ file trước khi in.

## 6. Cách nhập và ghi dữ liệu

### Nhập các trường văn bản

Với chuỗi có thể chứa dấu cách, dùng `fgets`:

```c
char ho_ten[TEXT_LEN];
printf("Ho va ten: ");
if (fgets(ho_ten, sizeof ho_ten, stdin) == NULL) {
    /* Xử lý lỗi nhập hoặc kết thúc luồng nhập. */
}
ho_ten[strcspn(ho_ten, "\n")] = '\0';
```

Nếu trước đó dùng `scanf` để đọc số, ký tự xuống dòng còn lại trong bộ đệm có thể làm `fgets` nhận chuỗi rỗng. Cần xử lý phần xuống dòng còn lại hoặc thống nhất dùng `fgets` cho mọi dữ liệu rồi chuyển đổi số.

### Tính tổng điểm

```c
student.tong = student.bai1 + student.bai2 + student.bai3;
```

### Mở file ở chế độ thêm

```c
FILE *file = fopen("students.txt", "a");
if (file == NULL) {
    perror("Khong mo duoc students.txt de ghi");
    return;
}
```

Chế độ `"a"` ghi thêm dữ liệu ở cuối file và giữ nguyên các bản ghi cũ. Nếu file chưa tồn tại, chế độ này thường tạo file mới. Dùng `"w"` sẽ xóa nội dung cũ khi mở, vì vậy không phù hợp với yêu cầu “ghi thêm”.

Ghi dữ liệu bằng `fprintf` theo đúng định dạng mà phần đọc hiểu:

```c
fprintf(file, "%d;%s;%s;%s;%s;%.2f;%.2f;%.2f;%.2f\n",
        student.thu_tu,
        student.sbd,
        student.ho_ten,
        student.ten_truong,
        student.ten_mon,
        student.bai1,
        student.bai2,
        student.bai3,
        student.tong);
```

Đây là ví dụ định dạng phân cách bằng dấu chấm phẩy. Nếu mã nguồn hiện tại của bạn dùng định dạng khác thì phần ghi và phần đọc phải thống nhất với định dạng đó. Dấu phân cách này chỉ an toàn khi giá trị các trường không chứa dấu `;`; nếu có thể xuất hiện, cần dùng định dạng có quy tắc trích dẫn/thoát ký tự hoặc định dạng khác.

Sau khi ghi, đóng file để đẩy dữ liệu còn trong bộ đệm ra file:

```c
fclose(file);
printf("Da ghi vao students.txt. Tong diem: %.2f\n", student.tong);
```

Ảnh chạy thử thể hiện thông báo tương ứng: `Da ghi vao students.txt. Tong diem: 27.00`. Khi chọn đọc lại, bản ghi số 89 xuất hiện phía sau các bản ghi trước đó, cho thấy thao tác ghi thêm đã giữ lại dữ liệu cũ.

### Ảnh minh họa: nhập và ghi thêm dữ liệu

Ảnh dưới đây ghi lại lần chọn chức năng `2`, nhập thông tin thí sinh mới, nhận thông báo tổng điểm và đọc lại file để kiểm tra bản ghi vừa thêm.

![Chương trình nhập và ghi thêm bản ghi vào students.txt](images/write_student.png)

## 7. Sơ đồ luồng dữ liệu

```text
Bàn phím
   │
   ├── Chọn 1 ──> mở students.txt bằng "r"
   │                └── đọc bản ghi ──> định dạng bảng ──> màn hình
   │
   └── Chọn 2 ──> nhập thông tin ──> tính tổng điểm
                    └── mở students.txt bằng "a"
                         └── ghi bản ghi mới ──> đóng file
```

## 8. Lưu ý khi vận hành

- Chạy chương trình từ thư mục chứa `students.txt`, hoặc cấu hình đường dẫn chính xác.
- Luôn kiểm tra `fopen` có trả về `NULL` hay không.
- Đóng file sau khi đọc hoặc ghi bằng `fclose`.
- Dùng `"a"` khi cần thêm bản ghi; dùng `"w"` sẽ ghi đè nội dung cũ.
- Thống nhất định dạng giữa phần ghi và phần đọc. Nếu ghi bằng dấu phân cách `;`, phần đọc cũng phải tách theo dấu `;`.
- Kiểm tra dữ liệu số trước khi ghi, chẳng hạn điểm không âm và nằm trong giới hạn mà bài toán quy định.
- Không dùng `%s` cho họ tên hoặc tên trường có dấu cách.
- Để hiển thị tiếng Việt đúng, lưu mã nguồn và file dữ liệu bằng UTF-8; cấu hình terminal cũng cần hỗ trợ UTF-8.
- Có thể kiểm tra nội dung file sau khi thêm bản ghi bằng:

```bash
cat students.txt
```

## 9. Tóm tắt

Chương trình dùng menu để chọn chức năng đọc hoặc ghi. Chức năng đọc mở `students.txt` bằng chế độ `"r"`, lấy các bản ghi và trình bày thành bảng. Chức năng ghi nhập thông tin mới, tính tổng ba điểm, mở file bằng chế độ `"a"` để nối bản ghi vào cuối, rồi đóng file. GCC biên dịch mã nguồn theo chuẩn C11 bằng lệnh `gcc -std=c11 -Wall -Wextra demo.c -o student`.

