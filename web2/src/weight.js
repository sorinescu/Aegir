/* Convert a weight in grams into a pretty representation */
export function weight(value) {
    value = parseFloat(value)
    if (!isFinite(value) || (!value && value !== 0)) return 'N/A'
    if (Math.abs(value) < 1000)
        return value.toFixed(0) + 'g'
    return (value / 1000).toFixed(2) + 'kg'
}
