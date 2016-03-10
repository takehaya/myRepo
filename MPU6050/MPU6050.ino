

/*TakeruHayasaka,20160302*/

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
// MPU control/status vars
MPU6050 mpu;
bool dmpReady = false;  // DMP の初期化が成功した場合は true に設定します。
uint8_t mpuIntStatus;   // MPU から実際の割り込みステータス バイトを保持します。
uint8_t devStatus;      // デバイス操作のたびに状態を返す(0 = success, !0 = error)
uint16_t packetSize;    // DMP パケット サイズ (デフォルトは 42 バイト)
uint16_t fifoCount;     // すべての FIFOのbyte の現在の値
uint8_t fifoBuffer[64]; // FIFO buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion
VectorInt16 aa;         // [x, y, z]            加速度センサー 計測値
VectorInt16 aaReal;     // [x, y, z]            重力加速度 計測値
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            重力ベクトル
float euler[3];         // [psi, theta, phi]    オイラー角
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
int16_t ax, ay, az;     //加速度センサー
int16_t gx, gy, gz;     //ジャイロセンサー

// ================================================================
// ===               割り込み検出ルーチン                         ===
// ================================================================

volatile bool mpuInterrupt = false;     // MPU割り込み端子がハイ・レベルになったかどうかを示します
void dmpDataReady() {
  mpuInterrupt = true;
}

void setup() {
  //i2c初期化
  Wire.begin();

  // シリアル通信を初期化します
  Serial.begin(115200);
  while (!Serial); // 立ち上がりまで待つ

  // デバイスの初期化
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // 接続の確認
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // 準備完了まで待機
  Serial.println(F("\nSend any character to begin DMP programming and demo: "));
  while (Serial.available() && Serial.read()); // empty buffer
  while (!Serial.available());                 // wait for data
  while (Serial.available() && Serial.read()); // empty buffer again

  // DMPの構成初期化
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // デバイスとのコネクションが成立しているか(0 = success, !0 = error)
  if (devStatus == 0) {
    // DMPを有効に
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    //Arduino の割り込みの検出を有効にします。
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // DMPが有効かというフラグ
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    //  DMP パケット サイズを取得します。
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

void loop() {

  // 失敗したらさよなら
  if (!dmpReady) return ;

  // −１wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize) {
    // other program behavior stuff here
    // .
    // .
    // .
    // if you are really paranoid you can frequently test in between other
    // stuff to see if mpuInterrupt is true, and if so, "break;" from the
    // while() loop to immediately process the MPU data
    // .
    // .
    // .
  }

  // 割り込みフラグをリセットし、INT_STATUS バイトを取得
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // 現在のFIFOの量を取得
  fifoCount = mpu.getFIFOCount();

  //  オーバーフローのチェック
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // オーバーフローリセット
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // それ以外の場合、DMP データ準備割り込み (頻繁に発生する必要あり) を確認します。
  } else if (mpuIntStatus & 0x02) {
    // 正確な利用可能なデータ長を待つ
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // FIFO からパケットを読む
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    //  1 パケットでもある場合ここで FIFO のカウントを追跡する。
    // (これにより、割り込みを待つことがなくすぐにできる)
    fifoCount -= packetSize;

    // オイラー角を度単位で表示
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    Serial.print("ypr\t");
    Serial.print(ypr[0] * 180 / M_PI);
    Serial.print("\t");
    Serial.print(ypr[1] * 180 / M_PI);
    Serial.print("\t");
    Serial.println(ypr[2] * 180 / M_PI);
    //生データ取得
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    Serial.print("a/g:\t");
    Serial.print(ax); Serial.print("\t");
    Serial.print(ay); Serial.print("\t");
    Serial.print(az); Serial.print("\t");
    Serial.print(gx); Serial.print("\t");
    Serial.print(gy); Serial.print("\t");
    Serial.println(gz);
}
}
