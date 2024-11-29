#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define I2C_BUS "/dev/i2c-1"  // I2C 버스 경로 (라즈베리파이에서는 /dev/i2c-1)
#define SENSOR_ADDR 0x53      // ADXL345 가속도 센서 I2C 주소 (센서에 따라 변경)

// 레지스터 주소
#define POWER_CTL 0x2D        // 전원 제어 레지스터
#define DATA_FORMAT 0x31      // 데이터 포맷 레지스터
#define DATAX0 0x32           // X축 데이터 시작 레지스터

int main() {
    int fd;  // I2C 디바이스 파일 디스크립터
    char buf[6];  // 센서 데이터 저장 버퍼

    // I2C 버스 열기
    if ((fd = open(I2C_BUS, O_RDWR)) < 0) {
        perror("Failed to open I2C bus");
        return 1;
    }

    // 센서 주소 설정
    if (ioctl(fd, I2C_SLAVE, SENSOR_ADDR) < 0) {
        perror("Failed to set I2C address");
        close(fd);
        return 1;
    }

    // 전원 제어: 센서 활성화
    char power_ctl[] = {POWER_CTL, 0x08};
    if (write(fd, power_ctl, 2) < 0) {
        perror("Failed to write to POWER_CTL");
        close(fd);
        return 1;
    }

    // 데이터 포맷 설정
    char data_format[] = {DATA_FORMAT, 0x08};
    if (write(fd, data_format, 2) < 0) {
        perror("Failed to write to DATA_FORMAT");
        close(fd);
        return 1;
    }

    // 가속도 데이터 읽기
    while (1) {
        // X, Y, Z 데이터 읽기 시작 주소 설정
        char reg = DATAX0;
        if (write(fd, &reg, 1) < 0) {
            perror("Failed to set data register");
            close(fd);
            return 1;
        }

        // 데이터 읽기 (X축, Y축, Z축 2바이트씩 총 6바이트)
        if (read(fd, buf, 6) < 0) {
            perror("Failed to read accelerometer data");
            close(fd);
            return 1;
        }

        // 16비트 데이터로 변환
        int x = (buf[1] << 8) | buf[0];
        int y = (buf[3] << 8) | buf[2];
        int z = (buf[5] << 8) | buf[4];

        // 출력
        printf("X: %d, Y: %d, Z: %d\n", x, y, z);
        usleep(500000);  // 0.5초 대기
    }

    close(fd);
    return 0;
}
