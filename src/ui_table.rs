use cursive::Vec2;
use cursive::theme::Color;
use cursive::theme::ColorStyle;
use cursive::theme::BaseColor;
use cursive::views::TextView;
use crate::sheet::Sheet;
use crate::field::Field;
use crate::field::FieldValue;
use crate::formula::FormulaData;


pub struct TableView<'a> {
    pub sheet: Sheet<'a>
}

impl cursive::view::View for TableView<'static> {
    
    fn draw(&self, printer: &cursive::Printer<'_, '_>) { 

        for (x,column) in self.sheet.data.iter().enumerate() {
            for (y,cell) in column.iter().enumerate() {
                match cell.value {
                    FieldValue::Text(str)=> {
                        printer.print((x*30, y), &str);
                    },
                    FieldValue::Number(n) => {
                        printer.print((x*30, y), &n.to_string());
                    },
                    FieldValue::Formula(FormulaData{value})=> {
                        printer.print((x*30, y), &value);
                    }
                    _ => {
                        printer.print((x*30, y), "__");
                    }
                };   
            } 
        }

        /*
        printer.with_color(
            ColorStyle::new(Color::Dark(BaseColor::Black), Color::RgbLowRes(3, 5, 3)),
            |printer| printer.print((20, 20), "Hello World"),
        );
        */

    }

    fn required_size(&mut self, _: Vec2) -> Vec2 {
        Vec2::new(200,40)
    }
} 