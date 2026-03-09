# Review tính năng macro UI / import-export / nhớ thư mục gần nhất

Ngày cập nhật: 2026-03-09  
Phạm vi: các thay đổi đã được mô tả trong planning từ `20260301` đến hiện tại và đã có trong branch làm việc.

---

## 1. Mục tiêu tài liệu

Tài liệu này giúp reviewer xem nhanh:
- **tính năng nào đã hoàn thành**,
- **test case nào tương ứng với từng tính năng**,
- **dữ liệu nào dùng để kiểm thử**,
- **kỳ vọng đầu ra** để xác nhận PR.

> Lưu ý: tài liệu này chỉ liệt kê **tính năng đã làm xong**. Các ý tưởng tương lai như drag/drop merge mode vẫn nằm ở tài liệu thiết kế riêng và **không phải phạm vi review chính của PR hiện tại**.

---

## 2. Dữ liệu test dùng cho review

### 2.1 Fixtures trong repo

- [unikey_macros.txt](../unikey_macros.txt) — dữ liệu macro UniKey gốc
- [unikey_macro.yaml](../unikey_macro.yaml) — dữ liệu Espanso YAML
- [unikey_macro.plist](../unikey_macro.plist) — dữ liệu thay thế văn bản của macOS

### 2.2 Test tự động

- [macro_dialog_state_test.cpp](../macro_dialog_state_test.cpp) — unit test cho remembered import/export directories bằng GSettings
- [README test UI/macros](../README.md) — hướng dẫn chạy test

---

## 3. Danh sách tính năng và test case

## Tính năng 1 — Bảng macro hỗ trợ quy mô lớn hơn trước

### Mô tả

- `CMacroTable` đã chuyển sang hướng quản lý động để xử lý tập macro lớn hơn.
- Giới hạn nội dung thay thế (`MAX_MACRO_TEXT_LEN`) đã tăng để phục vụ nội dung dài hơn.

### Test case đề xuất

**TC1.1 — Build và mở dialog với tập macro lớn**

**Bước thực hiện**
1. Build project.
2. Chạy `ibus-setup-unikey`.
3. Mở macro dialog.
4. Import file [unikey_macros.txt](../unikey_macros.txt).

**Kỳ vọng**
- Dialog mở bình thường.
- Không treo UI khi nạp dữ liệu.
- Danh sách hiển thị và cho phép thao tác tiếp.

**TC1.2 — Nội dung thay thế dài**

**Bước thực hiện**
1. Thêm mới một macro.
2. Dán một đoạn văn dài nhiều dòng vào ô `With`.
3. Nhấn `Save`.
4. Đóng và mở lại macro dialog.

**Kỳ vọng**
- Nội dung được lưu thành công.
- Mở lại vẫn thấy dữ liệu đúng.
- Không bị giới hạn kiểu single-line như trước.

---

## Tính năng 2 — Dictionary semantics khi load macro trùng key

### Mô tả

- Ở tầng engine / persisted storage, key trùng sẽ được chuẩn hóa theo nguyên tắc **bản xuất hiện sau thắng**.
- Điều này giúp macro file sau khi nạp có ngữ nghĩa kiểu dictionary.

### Test case đề xuất

**TC2.1 — File có key trùng**

**Bước thực hiện**
1. Tạo một file macro tạm với cùng một key xuất hiện hai lần, giá trị khác nhau.
2. Nạp file này qua engine hoặc import rồi save lại.
3. Kiểm tra giá trị cuối cùng được giữ lại.

**Kỳ vọng**
- Chỉ còn một key sau khi normalize.
- Giá trị cuối cùng trong file là giá trị có hiệu lực.

**Ghi chú cho reviewer**
- Trong planning đã nêu rõ engine và UI append/import có hai chính sách khác nhau.  
- Reviewer nên phân biệt rõ:
  - **engine load**: last occurrence wins,
  - **UI import append**: KeepExisting,
  - **dialog save/edit**: upsert/overwrite visible entry.

---

## Tính năng 3 — Dialog macro trở thành bề mặt CRUD chính

### Mô tả

- Dialog giờ chịu trách nhiệm load / edit / save toàn bộ vòng đời thao tác macro.
- Người dùng có thể thêm, sửa, xóa, clear, import, export trực tiếp từ giao diện.

### Test case đề xuất

**TC3.1 — Mở dialog và lưu thay đổi**

**Bước thực hiện**
1. Mở macro dialog.
2. Thêm hoặc sửa một macro.
3. Nhấn `Save` ở dialog chính.
4. Mở lại dialog.

**Kỳ vọng**
- Dữ liệu đã chỉnh sửa được persist xuống file macro người dùng.
- Mở lại thấy dữ liệu vừa lưu.

**TC3.2 — Nhấn Cancel ở dialog chính**

**Bước thực hiện**
1. Mở macro dialog.
2. Thực hiện vài thay đổi.
3. Nhấn `Cancel` / đóng dialog mà không chấp nhận.
4. Mở lại dialog.

**Kỳ vọng**
- Thay đổi chưa được lưu xuống file.
- Chỉ khi dialog chính trả `GTK_RESPONSE_OK` mới persist.

---

## Tính năng 4 — Add và Edit dùng chung một dialog

### Mô tả

- Dialog thêm macro đã được tái sử dụng cho chỉnh sửa entry hiện có.
- Nút xác nhận đã đổi sang `Save` thay vì chỉ mang nghĩa `Add`.

### Test case đề xuất

**TC4.1 — Add macro mới**

**Bước thực hiện**
1. Bấm `Add`.
2. Nhập key mới và value mới.
3. Nhấn `Save`.

**Kỳ vọng**
- Một entry mới xuất hiện trong danh sách.
- Entry được chọn lại và cuộn tới vị trí tương ứng sau khi sort.

**TC4.2 — Edit macro hiện có**

**Bước thực hiện**
1. Double-click một hàng đang có.
2. Dialog mở ra với dữ liệu được điền sẵn.
3. Sửa nội dung rồi nhấn `Save`.

**Kỳ vọng**
- Hàng cũ được cập nhật.
- Không tạo thêm bản sao ngoài ý muốn.

---

## Tính năng 5 — Không còn inline edit bằng double-click

### Mô tả

- Double-click và Enter trên row giờ mở dialog Add/Edit.
- Cell renderer đã bị ép về trạng thái inert / không editable trực tiếp.

### Test case đề xuất

**TC5.1 — Double-click không sửa trực tiếp cell**

**Bước thực hiện**
1. Mở macro dialog.
2. Double-click vào cột `Word` hoặc `Replace with`.

**Kỳ vọng**
- Không xuất hiện con trỏ chỉnh sửa inline trong cell.
- Dialog Add/Edit được mở.

**TC5.2 — Nhấn Enter trên row đang chọn**

**Bước thực hiện**
1. Chọn một row.
2. Nhấn `Enter`.

**Kỳ vọng**
- Dialog Add/Edit mở ra.
- Không có chỉnh sửa inline nào được kích hoạt.

---

## Tính năng 6 — Ô `With` hỗ trợ văn bản nhiều dòng, cuộn được

### Mô tả

- `replace/value` đã đổi từ `GtkEntry` sang `GtkTextView` bọc trong `GtkScrolledWindow`.
- Phù hợp hơn cho nội dung dài kiểu email, corporate text, đoạn hướng dẫn, v.v.

### Test case đề xuất

**TC6.1 — Nhập nhiều dòng**

**Bước thực hiện**
1. Mở dialog Add/Edit.
2. Nhập nhiều dòng có xuống dòng thật.
3. Nhấn `Save`.
4. Mở lại cùng entry.

**Kỳ vọng**
- Các dòng vẫn còn nguyên.
- Text view có thể cuộn nếu nội dung dài.

**TC6.2 — Dán đoạn văn dài**

**Bước thực hiện**
1. Copy một đoạn văn dài.
2. Paste vào `With`.
3. Kiểm tra khả năng cuộn và lưu.

**Kỳ vọng**
- Không bị cắt theo kiểu trường một dòng.
- Giao diện vẫn sử dụng được.

---

## Tính năng 7 — Save theo kiểu upsert / overwrite khi key trùng trong dialog

### Mô tả

- Khi người dùng `Save` từ dialog Add/Edit:
  - nếu key chưa tồn tại → thêm mới,
  - nếu key đã tồn tại → ghi đè entry hiện có,
  - sau đó danh sách được chọn lại và cuộn tới entry vừa thay đổi.

### Test case đề xuất

**TC7.1 — Thêm key trùng từ dialog**

**Bước thực hiện**
1. Tạo sẵn một macro với key `abc`.
2. Chọn `Add`.
3. Nhập lại key `abc` với value khác.
4. Nhấn `Save`.

**Kỳ vọng**
- Chỉ còn một key `abc` trong danh sách.
- Value là value mới.
- Danh sách cuộn tới đúng row đó.

**TC7.2 — Đổi key của entry hiện có thành key đã tồn tại**

**Bước thực hiện**
1. Có sẵn hai entry `a1` và `a2`.
2. Edit `a1`, đổi key thành `a2`.
3. Nhấn `Save`.

**Kỳ vọng**
- Hành vi là overwrite/upsert nhất quán.
- Không còn hai row cùng key sau thao tác.

---

## Tính năng 8 — Sort theo cột và giữ ngữ cảnh người dùng

### Mô tả

- Cột `Word` và `Replace with` có thể click để sort.
- Sentinel row luôn bị giữ ở cuối.
- Sau sort, UI cố giữ ngữ cảnh bằng cách scroll về selection hiện tại hoặc row đầu.

### Test case đề xuất

**TC8.1 — Sort theo Word**

**Bước thực hiện**
1. Mở dialog với nhiều row.
2. Click header `Word` nhiều lần.

**Kỳ vọng**
- Đổi qua lại ascending/descending.
- Mũi tên sort indicator hiển thị đúng.
- Sentinel row không nhảy lên trên dữ liệu thật.

**TC8.2 — Sort theo Replace with**

**Bước thực hiện**
1. Chọn một row bất kỳ.
2. Click header `Replace with`.

**Kỳ vọng**
- Sort theo nội dung value.
- Row đang chọn vẫn được giữ ngữ cảnh tốt hơn trước.

---

## Tính năng 9 — Import/export nhiều định dạng

### Mô tả

- Hỗ trợ import/export ít nhất ba dạng:
  - UniKey txt,
  - Espanso YAML,
  - macOS plist.
- Logic đọc/ghi được gom vào `macro_file_io`.

### Test case đề xuất

**TC9.1 — Import từ txt**

**Bước thực hiện**
1. Mở macro dialog.
2. Chọn `Import...`.
3. Import [unikey_macros.txt](../unikey_macros.txt).

**Kỳ vọng**
- Macro được đọc thành công.
- Số lượng row tăng đúng.
- Không crash / không lỗi parse.

**TC9.2 — Import từ yaml**

**Bước thực hiện**
1. Mở macro dialog.
2. Chọn `Import...`.
3. Import [unikey_macro.yaml](../unikey_macro.yaml).

**Kỳ vọng**
- Dữ liệu YAML được map đúng sang key/value.
- Entry hiển thị đúng trên UI.

**TC9.3 — Import từ plist**

**Bước thực hiện**
1. Mở macro dialog.
2. Chọn `Import...`.
3. Import [unikey_macro.plist](../unikey_macro.plist).

**Kỳ vọng**
- Dữ liệu plist được parse đúng.
- Không lỗi định dạng.

**TC9.4 — Export ra yaml / plist**

**Bước thực hiện**
1. Có sẵn một số macro trên UI.
2. Chọn `Export...`.
3. Lưu ra `.yaml` hoặc `.plist`.
4. Mở lại file bằng editor để kiểm tra sơ bộ.

**Kỳ vọng**
- Phần mở rộng xác định đúng format xuất.
- Nội dung file hợp lệ theo format tương ứng.

---

## Tính năng 10 — Nhớ thư mục import/export gần nhất bằng GSettings

### Mô tả

- Không dùng `config.yaml` riêng.
- Dùng `GSettings` key dùng chung:
  - `macro-last-working-dir`
- Nếu chưa có giá trị hoặc giá trị cũ không còn hợp lệ, ứng dụng tự fallback về
  thư mục mặc định `$HOME/Documents`.
- Khi mở lại file chooser, ứng dụng đưa người dùng về thư mục lần trước.
- Không thêm shortcut/sidebar tùy biến; chooser chỉ mở đúng thư mục đã nhớ.

### Test case đề xuất

**TC10.1 — Nhớ thư mục import**

**Bước thực hiện**
1. Mở `Import...` và chọn file trong một thư mục cụ thể.
2. Đóng dialog.
3. Mở lại `Import...`.

**Kỳ vọng**
- Chooser mở lại đúng thư mục vừa dùng.

**TC10.0 — Mặc định về Documents khi chưa có key**

**Bước thực hiện**
1. Dùng profile người dùng mới hoặc reset key GSettings liên quan.
2. Mở `Import...` hoặc `Export...` lần đầu.

**Kỳ vọng**
- Dialog bắt đầu ở `$HOME/Documents`.
- Key GSettings được seed về thư mục mặc định này.

**TC10.2 — Nhớ thư mục export**

**Bước thực hiện**
1. Mở `Export...` và lưu file trong một thư mục cụ thể.
2. Đóng dialog.
3. Mở lại `Export...`.

**Kỳ vọng**
- Chooser mở lại đúng thư mục export lần trước.

**TC10.3 — Import/export dùng chung thư mục làm việc gần nhất**

**Bước thực hiện**
1. Import từ thư mục A.
2. Export tới thư mục B.
3. Mở lại `Import...` rồi `Export...`.

**Kỳ vọng**
- `Import...` quay về B.
- `Export...` quay về B.
- Cả hai dialog đều dùng cùng thư mục làm việc gần nhất.

**TC10.4 — Shortcut sidebar xuất hiện khi thư mục còn tồn tại**

**Bước thực hiện**
1. Thực hiện import và export ở hai thư mục khác nhau.
2. Mở lại chooser.
3. Quan sát sidebar/shortcut folders.

**Kỳ vọng**
- Có shortcut tới các thư mục đã nhớ nếu còn tồn tại.

---

## Tính năng 11 — Unit test cho remembered chooser folders

### Mô tả

- Đã thêm test CTest cho phần nhớ thư mục import/export bằng GSettings backend kiểu memory.
- Test không ghi đè cấu hình desktop thật của người dùng.

### Test case đề xuất

**TC11.1 — Chạy test tự động**

**Lệnh**

```bash
ctest --test-dir build -R macro-dialog-state-test --output-on-failure
```

**Kỳ vọng**
- Test pass 100%.
- Kiểm tra được các trường hợp:
  - mặc định chưa có dữ liệu,
  - lưu parent directory của import,
  - lưu parent directory của export,
  - overwrite giá trị cũ,
  - import/export là hai vùng nhớ riêng.

---

## 4. Bộ lệnh reviewer nên chạy

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/setup/ibus-setup-unikey
```

---

## 5. Kịch bản review ngắn gọn được đề xuất

### Kịch bản A — review chức năng UI chính
1. Mở macro dialog.
2. Add một entry mới.
3. Double-click entry đó để sửa.
4. Dán đoạn text nhiều dòng dài.
5. Save.
6. Click sort ở hai cột.
7. Kiểm tra row vừa sửa được chọn và hiện ra đúng.

### Kịch bản B — review import/export format
1. Import lần lượt:
   - [unikey_macros.txt](../unikey_macros.txt)
   - [unikey_macro.yaml](../unikey_macro.yaml)
   - [unikey_macro.plist](../unikey_macro.plist)
2. Export ngược lại sang `.txt`, `.yaml`, `.plist`.
3. Mở file kết quả để kiểm tra sơ bộ.

### Kịch bản C — review nhớ thư mục gần nhất
1. Import từ một thư mục bất kỳ.
2. Export sang một thư mục khác.
3. Đóng/mở lại dialog.
4. Kiểm tra chooser có quay về đúng thư mục.

---

## 6. Ảnh minh họa

Thư mục [images](../images/README.md) hiện đã có một số ảnh minh họa phục vụ review nhanh.

### 6.1 Ảnh dialog macro

- Màn hình settings chính: [main-settings-window.png](../images/main-settings-window.png)
- Dialog macro tổng quan: [macro-dialog-main.png](../images/macro-dialog-main.png)
- Dialog thêm macro: [macro-dialog-add.png](../images/macro-dialog-add.png)
- Dialog sửa macro: [macro-dialog-edit.png](../images/macro-dialog-edit.png)

### 6.2 Ảnh file chooser / remembered last working dir

- Ví dụ chooser sau khi đã nhớ thư mục làm việc:
  - [macro-chooser-last-working-dir-example-1.png](../images/macro-chooser-last-working-dir-example-1.png)
  - [macro-chooser-last-working-dir-example-2.png](../images/macro-chooser-last-working-dir-example-2.png)
- Ảnh bổ sung cho luồng chooser:
  - [macro-chooser-example-3.png](../images/macro-chooser-example-3.png)
  - [macro-chooser-example-4.png](../images/macro-chooser-example-4.png)

### 6.3 Cách reviewer dùng ảnh

- Dùng nhóm ảnh dialog macro để đối chiếu bố cục Add/Edit và vùng nhập text nhiều dòng.
- Dùng nhóm ảnh chooser để đối chiếu hành vi nhớ thư mục làm việc gần nhất giữa import và export.
- Nếu cần ảnh/GIF khác sau này, chỉ cần đặt thêm file vào `tests/UI/macros/images/` và chèn link Markdown vào đây.

---

## 7. Kết luận ngắn cho reviewer

PR này nên được review như một gói thay đổi hoàn chỉnh cho subsystem macro, gồm:
- nâng cấp engine storage,
- nâng cấp CRUD dialog,
- hỗ trợ import/export đa định dạng,
- tăng khả năng nhập replacement text dài,
- bỏ inline edit để chuyển sang dialog nhất quán,
- ghi nhớ thư mục import/export gần nhất bằng GSettings,
- và bổ sung test/documentation để reviewer có thể xác nhận lại dễ dàng.
