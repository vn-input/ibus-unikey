## Hướng dẫn cách thêm một test case mới vào để test với UI automation test.
Mặc định, mọi thư mục trong **./tests** đều đươc xem là một suite riêng biệt (ngoại trừ thư mục pipeline dùng để chứa các script cài đặt và cấu hình máy ảo để test) và để đơn giản hóa, chúng ta chia các suite này theo các application cụ thể để test. 
Mỗi suite cần có thư mục **steps** để báo rằng đây là một suite chỉ dành cho UI test thôi. Bên trong steps sẽ chứa các bước cần để chạy test trên môi trường máy ảo được cài sẵn, chi tiết có thể tham khảo ví dụ mẫu về gedit.

## Lưu ý trước khi viết một test case mới
Test case hiện chỉ hỗ trợ dogtail vì hiện tại ubuntu latest không còn hỗ trợ các UI Automation khác như ldtp hay autopilot để test. Lý do chọn dogtail là vì hiện vẫn hỗ trợ tốt cho việc test các tác vụ tương đối đơn giản và chạy tốt trên cả gnome lẫn kde nên việc viết testcase sẽ đơn giản hơn.

## Cách để hỗ trợ một distro mới
 Mặc định, pipeline hiện chỉ hỗ trợ test trên ubuntu. Tuy nhiên, pipeline được thiết kế nhắm tới hỗ trợ cho nhiều distro khác nhau và với nhiều các khởi động khác nhau nhưng chủ yếu là dùng LiveCD để boot một máy ảo qemu không đĩa từ network và chạy test case. Mỗi distro cần được định nghĩa trong một file riêng trong thư mục ./tests/pipeline/environments với tên trùng với distro mà ta muốn hỗ trợ. 
 Mỗi file trong thư mục tests/pipeline/environments cần định nghĩa rõ các thành phần sau:
 - Hàm **fetch**: được gọi đầu tiên để download ISO image xuống máy test, pipeline tự sẽ biết để giải nén file ISO ra thành các phần cần thiết cho công việc chỉnh sửa và build một ISO image mới.
 - Hàm **extract**: được dùng để giải nén ra rootfs từ ISO image, mặc định các LiveCD đều chứa rootfs bên trong các file squashfs.
 - Hàm **configure**: được thực thi trực tiếp trên rootfs thông qua chroot để chỉnh sửa trực tiếp trên chính rootfs này. Ta có thể thêm bớt các thành phần cần thiết cho việc xây dựng một LiveCD riêng. Ở đây, pipeline chỉ cấu hình thêm dịch vụ ssh, cài dogtail cho việc test, cài password cho root là rootroot và cấu hình /etc/resolv.conf để dùng Google DNS. Lưu ý, luôn phải cấu hình account **root** là **rootroot** vì pipeline sẽ dùng account này để đăng nhập trực tiếp vào máy ảo để cấu hình LiveCD account mặc định.
 - Hàm **finish**: dùng để chỉnh sửa các phần không thuộc rootfs như thực hiện md5sum để cập nhật md5 mới của filesystem.squashfs hay cập nhật danh sách các deb đã được cài vào trong LiveCD.
 - Hàm **is_fully_started**: dùng để pipeline liên tục check vào máy ảo để xác định khi nào GUI đã thực sự sẵn sàng để test.
 - Hàm **version**: dùng để lấy mã latest version của distro tương ứng.
 - Hàm **username**: trả về username mặc định của LiveCD, với **ubuntu** account mặc định là **ubuntu** 
 - Hàm **password**: trả về password bạn muốn dùng cho account mặc định của LiveCD.
 - Hàm **interface**: trả về interface dùng để boot, hiện hàm này không thực sự cần thiết do nfs-ganesha gặp lỗi với nfs3 nên không thể dùng để boot các máy ảo lên.

#### Có thể tham khảo chi tiết qua file tests/pipeline/environments/ubuntu.

## Cách cấu hình một travis-ci mới
Bạn cần cung cấp các environment variables sau để pipeline có thể biết mà hỗ trợ bạn.
- PROJECT: tên của project hiện tại mặc định là ibus-unikey, biến này được dùng để truy cập vào storage để lấy các bản build tương ứng cho từng distro.
- MEGA: chứa username, password để đăng nhập, download và upload các bản build cho từng distro. Biến này nên được viết theo cấu trúc sau MEGA=email:password. Nên cấu hình biến này để tối ưu hóa quá trình chạy của pipeline vì khi đó các bản build sẽ được lưu vào mega.co.nz để tránh việc tốn thời gian cho việc build các file squashfs vốn tốn hầu như quá 50% thời gian chạy của pipeline.
- NGROK: chứa token của dịch vụ ngrok.com, pipeline nếu thấy biến môi trường này sẽ tự mở một localtunnel tới thẳng tới cổng VNC của máy ảo giúp bạn có thể theo dõi quá trình chạy của máy ảo.
- INTERACT: cờ dùng để khống chế pipeline sẽ tự thoát (set bằng 0) hay không (set lên 1)sau khi việc test hoàn tất. Mặc định là tự thoát.

#### Tất cả các biến môi trường này đều là optional nhưng nên cân nhắc cấu hình tùy theo tình huống cụ thể để giúp việc phát triển dự án tốt hơn.

## Các để kết nối VNC với pipeline
Có thể dùng http://allmydesktops.com/ để kết nối với vnc của máy ảo ngay khi pipeline mở localtunnel. Password của vnc luôn luôn là rootroot.