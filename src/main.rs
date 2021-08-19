#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_variables)]
use crate::ui_table::TableView;
use cursive::Vec2;
use cursive::theme::Color;
use cursive::theme::ColorStyle;
use cursive::theme::BaseColor;
use cursive::views::TextView;
use crate::field::FieldValue;
use crate::formula::FormulaData;
use rhai::{ASTNode, Engine, Expr, AST};
use smartstring;
use smartstring::Compact;
use std::error::Error;
use std::io;

mod field;
mod formula;
mod sheet;
mod ui_table;

fn main() -> Result<(), Box<dyn Error>> {
    let engine = Engine::new();
    //println!("Input a formula");
    // let mut input = String::new();
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
        //println!("{:?}", record);
    }
    let mut my_sheet = sheet::Sheet::create();
    my_sheet.data[0][0].value = FieldValue::Text("Hello");
    my_sheet.data[2][3].value = FieldValue::Number(2);
    my_sheet.data[2][1].value = FieldValue::Formula(FormulaData { value: "=x+2" });
    //println!("{:?}", my_sheet);
    init_ui();

    let mut siv = cursive::default();

	siv.add_global_callback('q', |s| s.quit());

	//siv.add_layer(TextView::new("Hello cursive! Press <q> to quit."));

    siv.add_layer(TableView{ sheet: my_sheet});

	siv.run();

    Ok(())
}

fn init_ui(){
    

}

fn collect_all_variables(ast: &AST) -> Vec<smartstring::SmartString<Compact>> {
    let mut result = Vec::new();
    ast.walk(&mut |path| -> bool {
        match path.last().expect("Always current node") {
            ASTNode::Expr(Expr::Variable(_, _, name)) => {
                let (_, _, id) = (**name).clone();
                result.push(id);
                true
            }
            _ => true,
        }
    });
    result
}




#[cfg(test)]
mod test {
    //  use super::*;
    #[test]
    fn noop() {
        assert!(true);
    }
}
