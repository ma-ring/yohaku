int mHR = 0;

void HRSetup() {
  // put your setup code here, to run once:
  // put your main code here, to run repeatedly:
  // それぞれの変数は下記のとおりとする
  float samplerate; //… サンプリング周波数
  float freq; //… カットオフ周波数
  float bw;  // … 帯域幅
  float omega = 2.0f * 3.14159265f *  freq / samplerate;
  float alpha = sin(omega) * sinh(log(2.0f) / 2.0 * bw * omega / sin(omega));

  float a0 =  1.0f + alpha;
  float a1 = -2.0f * cos(omega);
  float a2 =  1.0f - alpha;
  float b0 =  alpha;
  float b1 =  0.0f;
  float b2 = -alpha;
}

int HRLoop() {

  // それぞれの変数は下記のとおりとする
  float input[5];//  …入力信号の格納されたバッファ。
  float output[5];// …フィルタ処理した値を書き出す出力信号のバッファ。
  int   size;//     …入力信号・出力信号のバッファのサイズ。
  float in1, in2, out1, out2;//  …フィルタ計算用のバッファ変数。初期値は0。
  float a0, a1, a2, b0, b1, b2;// …フィルタの係数。 別途算出する。
  for (int i = 0; i < size; i++)
  {
    // 入力信号にフィルタを適用し、出力信号として書き出す。
    output[i] = b0 / a0 * input[i] + b1 / a0 * in1  + b2 / a0 * in2
                - a1 / a0 * out1 - a2 / a0 * out2;

    in2  = in1;       // 2つ前の入力信号を更新
    in1  = input[i];  // 1つ前の入力信号を更新

    out2 = out1;      // 2つ前の出力信号を更新
    out1 = output[i]; // 1つ前の出力信号を更新
  }

  return mHR;
}
