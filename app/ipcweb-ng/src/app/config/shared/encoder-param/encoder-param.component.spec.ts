import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { EncoderParamComponent } from './encoder-param.component';

describe('EncoderParamComponent', () => {
  let component: EncoderParamComponent;
  let fixture: ComponentFixture<EncoderParamComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ EncoderParamComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EncoderParamComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
