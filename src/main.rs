#![allow(dead_code)]
#![allow(unused_imports)]
use crate::field::FieldValue;
use smartstring::Compact;
use rhai::{ASTNode, Engine, Expr, AST};
use std::io;
use smartstring;
use std::error::Error;

mod formula;
mod field;
mod sheet;


fn main() -> Result<(), Box<dyn Error>> {
    let engine = Engine::new();
    println!("Input a formula");
    let mut input = String::new();
    //io::stdin()
    //    .read_line(&mut input)
    //    .expect("Failed to read input.");
    //let ast = engine.compile_expression(&input)?;
    //let all_vars = collect_all_variables(&ast);
    //println!("Your input {}", input);
    //println!("all vars {:#?}", all_vars);

    let mut rdr = csv::ReaderBuilder::new()
        .has_headers(false)
        .from_path("test.csv")
        .expect("Error open file");
    for result in rdr.records() {
        // The iterator yields Result<StringRecord, Error>, so we check the
        // error here.
        let record = result?;
        println!("{:?}", record);
    }
    let mut my_sheet = sheet::Sheet::create();
    my_sheet.data[0][0].value = FieldValue::Text("Hello");
    println!("{:?}", my_sheet);   

    Ok(())
}

fn collect_all_variables(ast: &AST) -> Vec<smartstring::SmartString<Compact>>
 {
    let mut result = Vec::new();
    ast.walk(&mut |path| -> bool {
        match path.last().expect("Always current node") {
            ASTNode::Expr(Expr::Variable(_, _, name)) => {
                let (_,_,id) = (**name).clone();
                result.push(id);
                true
            }
            _ => true
        }
    });
    result
}

#[cfg(test)]
mod test {
    //  use super::*;
    
    #[test]
    fn noop(){
        assert!(true);
    }
}
