use crate::raw_to_value;
use crate::field::{Field, FieldValue};


const MAX_SIZE:usize = 8; // 26 rows and columns will be enough for all

#[derive(Debug)]
pub struct Sheet<'a> {
    pub data: Vec<Vec<Field<'a>>>//[[Field<'a>; MAX_SIZE]; MAX_SIZE]
}

impl <'a> Sheet<'a> {
    pub fn create() -> Sheet<'a> {
        let mut data: Vec<Vec<Field>> = Vec::new();
        for _ in 0..MAX_SIZE {
            let mut vect: Vec<Field> = Vec::new();
            for _ in 0..MAX_SIZE {
                vect.push(Field{value: FieldValue::Empty()});
            }
            data.push(vect);
        }
        Sheet {
            data
        }
    }

   pub fn add(mut self:Self, value: FieldValue<'a>){
       self.data[0][0].value = value;
   } 
}



#[cfg(test)]
mod test {
    use super::*;
    
    #[test]
    fn can_create(){
        Sheet::create();
        assert!(true);
    }
}
