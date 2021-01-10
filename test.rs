//test
fn add(x:i32, y:i32) -> i32 { return x+y; };
fn wmain(){
    let mut b:i32 = 2 << 2;
    while b > 8{
        b += add(2,1);
    };
};

fn main(){
    let mut n:i32 = 5 + 3; 
    if n<6
    {
        n = n/2;
    }else{
        n = 5;
    };
};

fn fmain(){
    let mut a:f32 = 5.3 - 2.3;
    if a> 3.0
    {
        a = a* 2.5;
    }else{
        a = 4.5;
    };
}