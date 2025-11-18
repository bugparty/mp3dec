# MP3 解码器完整实现路线图

## 当前状态 ✅

### 已完成的模块
1. **MP3 帧解析** (mp3frame.c) - ✅ 完成
   - 帧同步和头解析
   - VBR (Xing/Info) 头解析
   - 比特率、采样率检测

2. **比特流处理** (bitstream.c) - ✅ 完成
   - 位级读取（1-bit, 9-bit, 17-bit）
   - 缓冲区管理

3. **I/O 系统** (io.c) - ✅ 完成
   - 文件读取和缓冲

4. **ID3 标签** (id3lib.c, id3Frame.c) - ✅ 基本完成
   - ID3v1/v2.3/v2.4 支持

5. **Layer III 部分功能** (layer3.c) - ⚠️ 部分完成
   - ✅ Side Info 解析
   - ✅ Scale Factors 解码（MPEG1 和 MPEG2/2.5）
   - ❌ 哈夫曼解码（未完成）
   - ❌ 反量化（未实现）
   - ❌ 重排序（未实现）
   - ❌ 立体声处理（未实现）
   - ❌ 混叠消除（未实现）
   - ❌ IMDCT（未实现）
   - ❌ 合成滤波器（未实现）

---

## 需要完成的工作 ❌

### 阻塞性问题（必须先修复）

#### 1. **修复 layer3.h 多重定义问题** - 🔴 Critical
**问题**：layer3.h 在多个 .c 文件中被包含，导致链接时多重定义错误

**影响**：无法链接生成完整的解码器可执行文件

**解决方案**：
```c
// 创建 layer3_tables.c 存放所有数组定义
// 修改 layer3.h：将所有定义改为 extern 声明

// layer3.h 中：
extern const float floatPow2[256];
extern const float floatPowIS[256];
extern const int intSfbIdxLong0[23];
// ... 等等

// layer3_tables.c 中：
const float floatPow2[256] = { /* 数据 */ };
const float floatPowIS[256] = { /* 数据 */ };
// ... 等等
```

**工作量**：2-3小时

---

#### 2. **修复帧同步问题** - 🔴 Critical
**问题**：只解析到 37 帧，应该有 8000+ 帧

**可能原因**：
- `frame_syncSearch()` 在第一次 64K 搜索失败后停止
- VBR 头解析后跳过了第一帧，但后续帧位置计算错误
- `io_dump()` 或 `io_seek()` 使用不当

**需要调试**：
- 添加详细日志输出每帧的位置
- 检查 VBR 头后的文件指针位置
- 验证帧大小计算是否正确

**工作量**：4-6小时

---

### 核心解码功能（按顺序实现）

#### 3. **实现哈夫曼解码** - 🟠 High Priority
**文件**：haffman.c

**当前状态**：
- ✅ 哈夫曼表初始化（已完成）
- ❌ Big Values 区域解码（未实现）
- ❌ Count1 区域解码（未实现）
- ❌ Linbits 扩展处理（未实现）

**需要实现的函数**：
```c
// 解码 big values 区域（使用哈夫曼表 0-31）
int huffman_decode_big_values(int region_start, int region_end,
                               PHuffTab htab, int *output);

// 解码 count1 区域（使用哈夫曼表 A 或 B）
int huffman_decode_count1(int count1_start, int count1_end,
                          PHuffTab htab, int *output);

// 处理 linbits 扩展
int process_linbits(int value, int linbits);
```

**参考标准**：ISO/IEC 11172-3 Section 2.4.3.4

**工作量**：10-15小时

---

#### 4. **实现反量化（Requantization）** - 🟠 High Priority
**文件**：layer3.c

**公式**（ISO/IEC 11172-3）：
```
对于 long blocks:
  xr[i] = sign(is[i]) * |is[i]|^(4/3) * 2^(global_gain/4 - 210)
         * 2^(scalefac * scalefac_scale)

对于 short blocks:
  xr[sb][i] = sign(is[sb][i]) * |is[sb][i]|^(4/3)
            * 2^(global_gain/4 - 210 - 8*subblock_gain[sb])
            * 2^(scalefac * scalefac_scale)
```

**需要实现**：
```c
void layer3_requantize(int ch, int gr);
```

**依赖**：需要 `floatPow2` 和 `floatPowIS` 查找表（已在 layer3.h）

**工作量**：8-12小时

---

#### 5. **实现重排序（Reordering）** - 🟡 Medium Priority
**文件**：layer3.c

**目的**：将 short blocks 的数据重新排列为正确的频域顺序

**需要实现**：
```c
void layer3_reorder(int ch, int gr);
```

**仅在**：`window_switching_flag == 1` 且 `block_type == 2` 时需要

**工作量**：4-6小时

---

#### 6. **实现立体声处理** - 🟡 Medium Priority
**文件**：layer3.c

**需要实现两种立体声模式**：

**MS Stereo (Middle-Side)**:
```c
void layer3_ms_stereo(int gr);
// M = (L + R) / sqrt(2)
// S = (L - R) / sqrt(2)
// 解码：L = (M + S) / sqrt(2), R = (M - S) / sqrt(2)
```

**Intensity Stereo**:
```c
void layer3_intensity_stereo(int gr);
// 使用 is_coef[] 或 lsf_is_coef[][] 表
```

**工作量**：6-8小时

---

#### 7. **实现混叠消除（Antialias）** - 🟡 Medium Priority
**文件**：layer3.c

**目的**：消除子带滤波器引入的混叠

**公式**：
```
for (sb = 1; sb < 32; sb++) {
    for (i = 0; i < 8; i++) {
        bu = xr[sb][17-i];
        bd = xr[sb-1][18+i];
        xr[sb][17-i] = bu * cs[i] - bd * ca[i];
        xr[sb-1][18+i] = bd * cs[i] + bu * ca[i];
    }
}
```

**需要实现**：
```c
void layer3_antialias(int ch, int gr);
```

**依赖**：`cs[]` 和 `ca[]` 数组（已在 layer3.h）

**工作量**：4-5小时

---

#### 8. **实现 IMDCT（逆修正离散余弦变换）** - 🔴 Critical
**文件**：新建 imdct.c

**这是最复杂的部分！**

**需要实现**：
```c
// 36点 IMDCT（用于 long blocks）
void imdct36(float *input, float *output, float *prev_block);

// 12点 IMDCT（用于 short blocks）
void imdct12(float *input, float *output, float *prev_block);

// 混合块 IMDCT
void imdct_mixed(float *input, float *output, float *prev_block);

// 窗口函数应用
void apply_window(float *samples, int block_type);
```

**需要的查找表**：
- 窗口函数（normal, start, stop, short）
- IMDCT 旋转因子（cos/sin 表）

**参考**：ISO/IEC 11172-3 Annex B.6

**工作量**：20-30小时（最耗时！）

---

#### 9. **实现频率倒置** - 🟢 Low Priority
**文件**：layer3.c

**简单的符号翻转**：
```c
void layer3_frequency_inversion(int ch, int gr) {
    for (int sb = 1; sb < 32; sb += 2) {
        for (int i = 1; i < 18; i += 2) {
            xr[ch][sb][i] = -xr[ch][sb][i];
        }
    }
}
```

**工作量**：1小时

---

#### 10. **实现合成滤波器组（Polyphase Synthesis Filterbank）** - 🔴 Critical
**文件**：新建 synthesis.c

**这是第二复杂的部分！**

**目的**：将 32 个子带转换为 32 个 PCM 样本

**需要实现**：
```c
// 初始化滤波器
void synthesis_init();

// 32子带合成滤波器
void synthesis_filter(float input[32], int channel, short *pcm_output);
```

**需要的查找表**：
- D[] 窗口系数（512 个值）
- V[] 向量缓冲区

**参考**：ISO/IEC 11172-3 Annex B.3

**工作量**：15-20小时

---

#### 11. **实现主解码循环** - 🟠 High Priority
**文件**：decoder.c（当前为空）

**需要实现完整的解码流程**：
```c
typedef struct {
    FILE *output_file;  // 输出 PCM 或 WAV 文件
    int channels;
    int sample_rate;
    // ... 其他状态
} MP3Decoder;

void decoder_init(MP3Decoder *dec, const char *filename);

int decoder_decode_frame(MP3Decoder *dec, short *pcm_samples);

void decoder_close(MP3Decoder *dec);

// 主解码循环
int main() {
    MP3Decoder dec;
    decoder_init(&dec, "input.mp3");

    short pcm_buffer[1152 * 2];  // 最大样本数 * 立体声

    while (decoder_decode_frame(&dec, pcm_buffer) > 0) {
        // 写入 PCM 数据到文件或播放
        fwrite(pcm_buffer, sizeof(short), samples, output);
    }

    decoder_close(&dec);
}
```

**每帧的解码步骤**：
1. 同步并解析帧头
2. 解析 Side Info
3. 读取主数据到比特流缓冲区
4. 对每个 granule (0-1)：
   - 对每个声道 (0-1)：
     - 解码 Scale Factors
     - 哈夫曼解码
     - 反量化
     - 重排序（如需要）
   - 立体声处理（如需要）
   - 对每个声道：
     - 混叠消除
     - IMDCT
     - 频率倒置
     - 合成滤波器
5. 输出 PCM 样本

**工作量**：8-10小时

---

### 其他改进

#### 12. **WAV 文件输出** - 🟢 Low Priority
**文件**：新建 wav.c

**实现 WAV 文件头写入**：
```c
void wav_write_header(FILE *f, int sample_rate, int channels, int num_samples);
```

**工作量**：2-3小时

---

#### 13. **内存泄漏修复** - 🟡 Medium Priority
**已知问题**：
- haffman.c 中的 malloc 没有对应的 free
- layer3.c 中的 si 指针
- VBR TOC 缓冲区

**工作量**：3-4小时

---

#### 14. **错误处理改进** - 🟢 Low Priority
- 添加 malloc 失败检查
- 文件读取错误处理
- 不支持的 MPEG 层处理

**工作量**：4-5小时

---

## 总工作量估算

### 阻塞性问题
- layer3.h 重构：2-3小时
- 帧同步修复：4-6小时
**小计**：6-9小时

### 核心解码功能
- 哈夫曼解码：10-15小时
- 反量化：8-12小时
- 重排序：4-6小时
- 立体声处理：6-8小时
- 混叠消除：4-5小时
- IMDCT：20-30小时 ⚠️
- 频率倒置：1小时
- 合成滤波器：15-20小时 ⚠️
- 主解码循环：8-10小时
**小计**：76-107小时

### 其他改进
- WAV 输出：2-3小时
- 内存泄漏：3-4小时
- 错误处理：4-5小时
**小计**：9-12小时

---

## **总计：91-128 小时** (约 11-16 个工作日)

---

## 推荐实施顺序

### Phase 1: 修复阻塞问题 (1-2天)
1. ✅ 修复 layer3.h 多重定义
2. ✅ 修复帧同步bug
3. ✅ 验证能正确读取所有帧

### Phase 2: 数据流解码 (3-5天)
4. ✅ 实现哈夫曼解码
5. ✅ 实现反量化
6. ✅ 实现重排序
7. ✅ 创建测试程序验证数据解码

### Phase 3: 频域处理 (2-3天)
8. ✅ 实现立体声处理
9. ✅ 实现混叠消除
10. ✅ 实现频率倒置

### Phase 4: 时域转换 (4-6天) ⚠️ 最难
11. ✅ 实现 IMDCT（最复杂！）
12. ✅ 实现合成滤波器（第二复杂！）
13. ✅ 验证输出 PCM 数据

### Phase 5: 完善和优化 (1-2天)
14. ✅ 实现主解码循环
15. ✅ 添加 WAV 文件输出
16. ✅ 修复内存泄漏
17. ✅ 完整测试

---

## 参考资料

### 必读文档
1. **ISO/IEC 11172-3** - MPEG-1 Audio Standard
2. **ISO/IEC 13818-3** - MPEG-2 Audio Standard
3. **Hydrogenaudio MP3 Wiki**
4. **The LAME Project** - 参考实现

### 推荐开源实现（学习参考）
- **libmad** - 高质量参考实现
- **minimp3** - 极简实现，适合学习
- **mpg123** - 另一个流行实现

---

## 简化建议

如果时间有限，可以考虑：

### 最小可行解码器（MVD）
只实现 MPEG-1, Layer III, 单声道或普通立体声，跳过：
- Intensity Stereo
- MS Stereo
- Mixed blocks
- 所有 MPEG-2/2.5 特性

**减少工作量到**：约 60-80 小时

### 或者，使用现有库
- 链接 libmad 或 minimp3
- 你的项目专注于：
  - MP3 文件解析（已完成）✅
  - ID3 标签处理（已完成）✅
  - 播放器界面
  - 可视化效果

---

## 结论

**当前项目状态**：约 40% 完成（解析和部分解码准备）

**完整解码所需**：
- ✅ 修复 2 个阻塞性bug
- ❌ 实现 9 个核心解码功能
- ❌ 总计 ~100 小时工作量

**IMDCT 和合成滤波器是最大挑战**，占总工作量的 35-50%！

你打算继续完成完整解码，还是采用简化方案？
