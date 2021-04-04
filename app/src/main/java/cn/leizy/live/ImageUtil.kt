package cn.leizy.live

/**
 * @author wulei
 * @date 4/3/21
 * @description
 */
object ImageUtil {
    fun yuvToNv21(
        y: ByteArray,
        u: ByteArray,
        v: ByteArray,
        nv21: ByteArray,
        stride: Int,
        height: Int
    ) {
        System.arraycopy(y, 0, nv21, 0, y.size)
        val length = y.size + u.size / 2 + v.size / 2
        var uIndex = 0
        var vIndex = 0
        for (i in stride * height until length step 2) {
            nv21[i] = v[vIndex]
            nv21[i + 1] = u[uIndex]
            vIndex += 2
            uIndex += 2
        }
    }

    fun nv21_rotate_to_90(
        nv21_data: ByteArray,
        nv21_rotated: ByteArray,
        width: Int,
        height: Int
    ): ByteArray {
        val y_size = width * height
        val buffer_size = y_size * 3 / 2
        var i = 0
        val startPos = (height - 1) * width
        for (x in 0 until width) {
            var offset = startPos
            for (y in height - 1 downTo 0) {
                nv21_rotated[i] = nv21_data[offset + x]
                i++
                offset -= width
            }
        }
        i = buffer_size - 1
        for (x in width - 1 downTo 1 step 2) {
            var offset = y_size
            for (y in 0 until height / 2) {
                nv21_rotated[i] = nv21_data[offset + x]
                i--
                nv21_rotated[i] = nv21_data[offset + (x - 1)]
                i--
                offset += width
            }
        }
        return nv21_rotated
    }
}