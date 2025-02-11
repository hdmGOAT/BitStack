# BitStack

**BitStack** is a simple **bit-stacking encoder and decoder** that rearranges bits in layers, allowing better data redundancy handling for compressors.

## How It Works
### **Encoding**
- Bits from the original file are distributed into separate layers based on their position.

### **Decoding**
- The layers are recombined to reconstruct the original file.
