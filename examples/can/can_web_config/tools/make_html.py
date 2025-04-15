# 파일 경로 지정
input_file_path = "setting_page.html"  # 입력 파일 경로
output_file_path = "output_hex.txt"    # 출력 파일 경로

# HTML 파일을 열고 내용 읽기
with open(input_file_path, 'r', encoding='utf-8') as file:
    html_content = file.read()

# html_content를 바이트 형태로 변환
html_bytes = html_content.encode('utf-8')

# Hex 값으로 변환
hex_values = [f"0x{byte:02X}" for byte in html_bytes]

# 결과를 txt 파일로 저장
with open(output_file_path, 'w', encoding='utf-8') as output_file:
    output_file.write(f"static const uint8_t _acindex[{len(hex_values)}] = {{\n")
    for i, hex_value in enumerate(hex_values):
        if (i + 1) % 12 == 0:  # 12개마다 줄바꿈
            output_file.write(f"  {hex_value},\n")
        else:
            output_file.write(f"  {hex_value}, ")
    output_file.write("\n};\n")

print(f"변환된 결과가 '{output_file_path}' 파일에 저장되었습니다.")
