#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define I2C_BUS "/dev/i2c-1"  // I2C 버스 경로
#define MPU6050_ADDR 0x68     // MPU6050 디바이스 주소
#define PWR_MGMT_1 0x6B       // 전원 관리 레지스터
#define ACCEL_XOUT_H 0x3B     // 가속도 데이터 시작 레지스터

int main() {
    int fd; // I2C 디바이스 파일 디스크립터
    char buf[6]; // 데이터 버퍼

    // 1. I2C 버스 열기
    if ((fd = open(I2C_BUS, O_RDWR)) < 0) {
        perror("Failed to open the I2C bus");
        return 1;
    }

    // 2. MPU6050 디바이스 주소 설정
    if (ioctl(fd, I2C_SLAVE, MPU6050_ADDR) < 0) {
        perror("Failed to connect to the sensor");
        close(fd);
        return 1;
    }

    // 3. MPU6050 초기화: PWR_MGMT_1 레지스터에 0x00 기록 (슬립 모드 해제)
    char config[2];
    config[0] = PWR_MGMT_1;
    config[1] = 0x00;
    if (write(fd, config, 2) < 0) {
        perror("Failed to initialize MPU6050");
        close(fd);
        return 1;
    }

    // 4. 가속도 데이터 읽기 루프
    while (1) {
        // 가속도 데이터 시작 주소 설정
        char reg = ACCEL_XOUT_H;
        if (write(fd, &reg, 1) < 0) {
            perror("Failed to set data register");
            close(fd);
            return 1;
        }

        // X, Y, Z 축 데이터 읽기 (6바이트)
        if (read(fd, buf, 6) < 0) {
            perror("Failed to read accelerometer data");
            close(fd);
            return 1;
        }

        // X, Y, Z 데이터 변환 (16비트 값)
        int16_t accel_x = (buf[0] << 8) | buf[1];
        int16_t accel_y = (buf[2] << 8) | buf[3];
        int16_t accel_z = (buf[4] << 8) | buf[5];

        // 가속도 출력
        printf("Accel X: %d, Accel Y: %d, Accel Z: %d\n", accel_x, accel_y, accel_z);

        usleep(500000); // 0.5초 대기
    }

    close(fd);
    return 0;
}
