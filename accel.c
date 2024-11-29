#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>       // sqrt 사용을 위해 추가
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define I2C_DEV "/dev/i2c-1"   // I²C 디바이스 파일
#define SENSOR_ADDR 0x68       // 가속도 센서 주소 (예: MPU6050)
#define ACCEL_XOUT_H 0x3B      // X축 데이터 레지스터 (MSB 시작)
#define ACCEL_YOUT_H 0x3D      // Y축 데이터 레지스터 (MSB 시작)
#define ACCEL_ZOUT_H 0x3F      // Z축 데이터 레지스터 (MSB 시작)

#define THRESHOLD 2.0          // 임계값 (m/s²)
#define SENSITIVITY 16384.0    // 가속도 센서의 감도 (1g = 16384)

int i2c_fd;

// I²C 초기화
int I2CInit() {
    if ((i2c_fd = open(I2C_DEV, O_RDWR)) < 0) {
        perror("Failed to open I2C device");
        return -1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, SENSOR_ADDR) < 0) {
        perror("Failed to connect to sensor");
        return -1;
    }

    return 0;
}

// 가속도 센서에서 데이터 읽기
short readAccelData(int reg) {
    unsigned char buf[2];
    if (write(i2c_fd, &reg, 1) != 1) {
        perror("Failed to write to sensor");
        return -1;
    }

    if (read(i2c_fd, buf, 2) != 2) {
        perror("Failed to read from sensor");
        return -1;
    }

    return (buf[0] << 8) | buf[1];  // 두 바이트를 합쳐 16비트 데이터 생성
}

// X, Y, Z 축 가속도 읽기
void readAccelXYZ(float *x, float *y, float *z) {
    *x = (float)readAccelData(ACCEL_XOUT_H) / SENSITIVITY;
    *y = (float)readAccelData(ACCEL_YOUT_H) / SENSITIVITY;
    *z = (float)readAccelData(ACCEL_ZOUT_H) / SENSITIVITY;
}

// 가속도 벡터의 크기 계산
float calculateMagnitude(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

int main() {
    if (I2CInit() == -1) {
        return -1;
    }

    clock_t start_t, end_t;
    double duration;
    int over_threshold = 0;  // 임계값 초과 상태 플래그

    while (1) {
        float accel_x, accel_y, accel_z;
        readAccelXYZ(&accel_x, &accel_y, &accel_z);  // X, Y, Z축 가속도 읽기

        float magnitude = calculateMagnitude(accel_x, accel_y, accel_z);  // 합성 가속도 계산
        printf("Accel Magnitude: %.2f m/s² (X: %.2f, Y: %.2f, Z: %.2f)\n",
               magnitude, accel_x, accel_y, accel_z);

        if (magnitude > THRESHOLD) {
            if (!over_threshold) {  // 임계값 초과 시작
                start_t = clock();
                over_threshold = 1;
                printf("Threshold exceeded, starting timer...\n");
            }
        } else {
            if (over_threshold) {  // 임계값 초과 종료
                end_t = clock();
                duration = (double)(end_t - start_t) / CLOCKS_PER_SEC;
                printf("Threshold exceeded for %.4f seconds\n", duration);
                over_threshold = 0;
            }
        }

        usleep(100000);  // 100ms 대기
    }

    close(i2c_fd);  // I²C 디바이스 닫기
    return 0;
}
