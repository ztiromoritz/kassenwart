use crate::formula::FormulaData;

#[derive(Debug)]
pub struct Field <'a> {
    pub value: FieldValue<'a>,
}

#[derive(PartialEq)]
#[derive(Debug)]
pub enum FieldValue<'a> {
    Empty(),
    Number(i32),
    Text(&'a str),
    Formula(FormulaData<'a>),
}

pub fn raw_to_value(raw_value: &str) -> FieldValue {
    if raw_value.is_empty() {
        return FieldValue::Empty()
    }else if raw_value.starts_with("=") {
        return FieldValue::Formula(FormulaData {
            value : &raw_value    
        })
    }else {
        match raw_value.parse::<i32>() {
            Ok(n) => FieldValue::Number(n),
            Err(_) => FieldValue::Text(&raw_value),
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    #[test]
    fn empty_field() {
        assert!(matches!(raw_to_value(""), FieldValue::Empty { .. }))
    }

    #[test]
    fn number_field() {
        assert!(matches!(raw_to_value("22"), FieldValue::Number(22)))
    }

    #[test]
    fn text_filed() {
        assert!(matches!(raw_to_value("adjkf"), FieldValue::Text { .. }))
    }

    #[test]
    fn formula_field() {
        assert!(matches!(raw_to_value("=33+x"), FieldValue::Formula { .. }))
    }
}
