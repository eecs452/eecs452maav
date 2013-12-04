
typedef union _Color_u{
    int8_t raw;
    struct {
        int8_t strength :5;
        uint8_t R :1;
        uint8_t G :1;
        uint8_t B :1;
    } f;
} Color_u;
